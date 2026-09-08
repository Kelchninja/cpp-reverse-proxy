#pragma once
#include "httprequestbuilder.hpp"
#include <asio/streambuf.hpp>
#include <memory>
#include <llhttp.h>
#include <asio.hpp>

namespace server
{
    
    class Connection : std::enable_shared_from_this<Connection>
    {
        public:
        Connection(asio::ip::tcp::socket);
        void start();

        private:
        void read();
        static int on_url(llhttp_t*, const char*, size_t);
        static int on_header_field(llhttp_t*, const char*, size_t);
        static int on_header_value(llhttp_t*, const char*, size_t);
        static int on_body(llhttp_t*, const char*, size_t);
        static int on_message_completed(llhttp_t*);

        asio::ip::tcp::socket socket_;
        asio::streambuf buf_;
        llhttp_t parser_;
        llhttp_settings_t settings_;
        std::string current_field_;
        http::HttpRequestBuilder builder_;

    };

}