#pragma once

#include "config.hpp"
#include "gateway.hpp"
#include <asio.hpp>
#include <asio/io_context.hpp>
#include <thread>
#include <vector>

namespace server
{

    class Server
    {
        public:
        Server(asio::io_context&, 
            asio::ip::tcp::acceptor,
            gateway::Gateway&, 
            const config::ServerConfig&);
        virtual ~Server();

        void init();
        void run();
        void wait();
        void stop();
        
        protected:
        virtual asio::awaitable<void> process();
        virtual asio::awaitable<void> handleClient(asio::ip::tcp::socket);

        private:
        enum class State { uninitialized, initialized, running, stopped } state_;
        std::vector<std::jthread> threads_;
        asio::io_context& io_context_;
        asio::ip::tcp::acceptor acceptor_;
        gateway::Gateway& gateway_;
        config::ServerConfig config_;
    };

}