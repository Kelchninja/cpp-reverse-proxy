#include "server.hpp"
#include "config.hpp"
#include "connection.hpp"
#include "request.hpp"
#include "gateway.hpp"
#include <asio/awaitable.hpp>
#include <asio/basic_stream_socket.hpp>
#include <asio/co_spawn.hpp>
#include <asio/detached.hpp>
#include <asio/io_context.hpp>
#include <asio/ip/tcp.hpp>
#include <asio/streambuf.hpp>
#include <asio/this_coro.hpp>
#include <cstddef>
#include <exception>
#include <stdexcept>
#include <thread>

namespace {

    size_t get_available_cores() {
        size_t hc{std::thread::hardware_concurrency()};
        return hc > 0 ? hc : 1;
    }

}

using namespace server;

Server::Server(asio::io_context& io_context, asio::ip::tcp::acceptor acceptor, const config::ServerConfig& config)
: state_{State::uninitialized}, io_context_{io_context}, acceptor_{std::move(acceptor)}, config_{config} {}

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

    if (state_ == State::running)
    {
        return;
    }

    if (state_ == State::stopped)
    {
        io_context_.restart();
        threads_.clear();
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
        std::println("received a connection");

        asio::co_spawn(socket.get_executor(), handleClient(std::move(socket)), 
        asio::detached);
    }
}


asio::awaitable<void> handleClient(asio::ip::tcp::socket socket)
{
    auto connection{std::make_shared<Connection>(std::move(socket))};

    // keep-alive
    while (true)
    {
        try
        {
            http::HttpRequest req{co_await connection->async_read_request()};
            gateway::send_to_backend(req);
        } 
        catch (const std::exception&)
        {
            break;
        }
    }
}

