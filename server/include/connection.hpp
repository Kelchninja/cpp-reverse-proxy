#pragma once
#include "request.hpp"
#include "response.hpp"
#include "serializer.hpp"
#include <asio/awaitable.hpp>
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
        asio::awaitable<http::HttpRequest> async_read_request();
        asio::awaitable<void> send_response(const http::HttpResponse&);
        asio::ip::tcp::endpoint remote_endpoint() const;

        private:
        static int on_url(llhttp_t*, const char*, size_t);
        static int on_header_field(llhttp_t*, const char*, size_t);
        static int on_header_value(llhttp_t*, const char*, size_t);
        static int on_header_value_complete(llhttp_t*);
        static int on_body(llhttp_t*, const char*, size_t);
        static int on_message_complete(llhttp_t*);

        asio::ip::tcp::socket socket_;
        asio::streambuf buf_;
        llhttp_t parser_;
        llhttp_settings_t settings_;
        http::HttpSerializer serializer_;
        std::string current_header_field_;
        std::string current_header_value_;

        // TODO: implement this shit
        // processing::processingConfig config_;

        bool request_ready_ = false;
        std::optional<http::HttpRequest> current_request_;

    };

    
    class UpstreamConnection : public std::enable_shared_from_this<UpstreamConnection>
    {
        public:
        UpstreamConnection(asio::ip::tcp::socket);
        asio::awaitable<void> send_request(const http::HttpRequest&);
        asio::awaitable<http::HttpResponse> async_read_response();

        private:
        static int on_status(llhttp_t*, const char*, size_t);
        static int on_header_field(llhttp_t*, const char*, size_t);
        static int on_header_value(llhttp_t*, const char*, size_t);
        static int on_header_value_complete(llhttp_t*);
        static int on_body(llhttp_t*, const char*, size_t);
        static int on_message_complete(llhttp_t*);

        asio::ip::tcp::socket socket_;
        asio::streambuf buf_;
        llhttp_t parser_;
        llhttp_settings_t settings_;
        http::HttpSerializer serializer_;
        std::string current_header_field_;
        std::string current_header_value_;

        bool response_ready_ = false;
        std::optional<http::HttpResponse> current_response_;

    };


}



