#pragma once

#include "config.hpp"
#include <asio.hpp>
#include <asio/io_context.hpp>
#include <thread>
#include <vector>

namespace server
{

    class Server
    {
        public:
        Server(asio::io_context&, asio::ip::tcp::acceptor, const config::ServerConfig&);
        virtual ~Server();

        void init();
        void run();
        void wait();
        void stop();
        asio::awaitable<void> process();
        
        private:
        enum {uninitialized, initialized, running, stopped} state_;
        std::vector<std::jthread> threads_;
        asio::io_context& io_context_;
        asio::ip::tcp::acceptor acceptor_;
        config::ServerConfig config_;
    };

}