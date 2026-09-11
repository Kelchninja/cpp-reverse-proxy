#pragma once
#include "config.hpp"
#include "request.hpp"
#include "response.hpp"
#include <asio/awaitable.hpp>
#include <asio/ip/tcp.hpp>

namespace gateway
{
    class Gateway
    {
        public:
        explicit Gateway(const config::GatewayConfig&);
        asio::awaitable<http::HttpResponse> send_to_backend(http::HttpRequest req, asio::ip::tcp::endpoint client);

        private:

        const config::GatewayConfig& config_;
        asio::ip::tcp::endpoint endpoint_;
    };
}


