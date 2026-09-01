#pragma once

#include "config.hpp"
#include <asio.hpp>
#include <asio/io_context.hpp>

namespace server
{

    class Server
    {
        public:
        Server(asio::io_context, asio::ip::tcp::acceptor, const config::ServerConfig&);
        void init();
        void run();
        void stop();
        asio::awaitable<void> process();
        protected:
        private:
        asio::io_context& io_context;
        asio::ip::tcp::acceptor acceptor;
    };

}