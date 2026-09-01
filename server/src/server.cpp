#include "server.hpp"
#include "config.hpp"
#include <asio/detached.hpp>
#include <asio/io_context.hpp>
#include <asio/ip/tcp.hpp>

using namespace server;

// Server::Server(asio::io_context io_context, asio::ip::tcp::acceptor acceptor, const config::ServerConfig& config)
// : 

void Server::Server::init()
{
    asio::co_spawn(io_context, process(), asio::detached);
}

void Server::Server::run()
{
    for (auto i{0uz}; i < config.max_threads)
}

// asio::awaitable<void> Server::Server::process()
// {
//     acceptor.
// }

