#include "common.h"
#include "server.hpp"
#include "config.hpp"
#include <asio/awaitable.hpp>
#include <asio/detached.hpp>
#include <asio/io_context.hpp>
#include <asio/ip/tcp.hpp>
#include <cstddef>
#include <thread>

namespace {

    size_t get_available_cores() {
        size_t hc{std::thread::hardware_concurrency()};
        return hc > 0 ? hc : 1;
    }

    // using asio::awaitable;

}

using namespace server;

Server::Server(asio::io_context& io_context, asio::ip::tcp::acceptor acceptor, const config::ServerConfig& config)
: io_context_{io_context}, acceptor_{std::move(acceptor)}, config_{config} {}

Server::~Server()
{
    stop();
}

void Server::Server::init()
{
    asio::co_spawn(io_context_, process(), asio::detached);
}

void Server::run()
{
    if (state_ == running)
    {
        return;
    }

    if (state_ == stopped)
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
}

void Server::stop()
{
    io_context_.stop();
}

// awaitable<void> Server::process()
// {

// }


