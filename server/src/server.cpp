#include "server.hpp"
#include "config.hpp"
#include "connection.hpp"
#include "request.hpp"
#include "gateway.hpp"
#include "response.hpp"
#include <asio/awaitable.hpp>
#include <asio/basic_stream_socket.hpp>
#include <asio/co_spawn.hpp>
#include <asio/detached.hpp>
#include <asio/io_context.hpp>
#include <asio/ip/tcp.hpp>
#include <asio/streambuf.hpp>
#include <asio/this_coro.hpp>
#include <algorithm>
#include <cctype>
#include <cstddef>
#include <exception>
#include <print>
#include <stdexcept>
#include <string>
#include <thread>

namespace {

    size_t get_available_cores() {
        size_t hc{std::thread::hardware_concurrency()};
        return hc > 0 ? hc : 1;
    }

    // HTTP/1.0 ist per Default nicht persistent, HTTP/1.1 nur bei explizitem "close"
    bool client_wants_close(const http::HttpRequest& req)
    {
        if (req.version.version_major == 1 && req.version.version_minor_ == 0) return true;

        for (const auto& [name, value] : req.headers)
        {
            if (name != "connection") continue;

            std::string lowered{value};
            std::transform(lowered.begin(), lowered.end(), lowered.begin(),
                [](unsigned char c) { return std::tolower(c); });
            if (lowered.find("close") != std::string::npos) return true;
        }
        return false;
    }

    http::HttpResponse bad_gateway()
    {
        return http::HttpResponse{
            .status_code = 502,
            .status_message = "Bad Gateway",
            .version = {1, 1},
            .headers = {{"content-length", "0"}},
            .body = {}
        };
    }

}

using namespace server;

Server::Server(asio::io_context& io_context, asio::ip::tcp::acceptor acceptor, gateway::Gateway& gateway, const config::ServerConfig& config)
: state_{State::uninitialized}, io_context_{io_context}, acceptor_{std::move(acceptor)}, gateway_{gateway}, config_{config} {}

Server::~Server()
{
    stop();
}

void Server::Server::init()
{
    if (state_ != State::uninitialized)
    {
        throw std::runtime_error("Server is already initialized");
    }
    asio::co_spawn(io_context_, process(), asio::detached);
    state_ = State::initialized;
}

void Server::run()
{
    switch (state_)
    {
        case State::uninitialized:
        {
            throw std::runtime_error("Server is not initialized");
            break;
        }
        case State::running: return;
        case State::stopped:
        {
            io_context_.restart();
            threads_.clear();
            break;
        }

        default:
            break;
    }

    size_t used_cores{config_.auto_detect_cores ? get_available_cores() : config_.available_cores};

    threads_.reserve(used_cores);
    for (auto i{0uz}; i < used_cores; ++i)
    {
        threads_.emplace_back([this](){ io_context_.run(); });
    }

    state_ = State::running;
}

void Server::wait()
{
    for (auto &t : threads_)
    {
        if (t.joinable()) t.join();
    }
}

void Server::stop()
{
    io_context_.stop();
    state_ = State::stopped;
}

asio::awaitable<void> Server::process()
{
    while (true)
    {
        auto socket{co_await acceptor_.async_accept()};
        // std::println("received a connection");

        asio::co_spawn(socket.get_executor(), handleClient(std::move(socket)), 
        asio::detached);
    }
}


asio::awaitable<void> Server::handleClient(asio::ip::tcp::socket socket)
{
    auto connection{std::make_shared<Connection>(std::move(socket))};

    // keep-alive
    while (true)
    {
        http::HttpRequest req;
        try
        {
            req = co_await connection->async_read_request();
        }
        catch (const std::exception&)
        {
            break;
        }

        const bool close_after{client_wants_close(req)};

        http::HttpResponse res;
        try
        {
            res = co_await gateway_.send_to_backend(std::move(req), connection->remote_endpoint());
        }
        catch (const std::exception&)
        {
            res = bad_gateway();
        }

        if (close_after)
        {
            res.headers.emplace_back("connection", "close");
        }

        try
        {
            co_await connection->send_response(res);
        }
        catch (const std::exception&)
        {
            break;
        }

        if (close_after) break;
    }
}

