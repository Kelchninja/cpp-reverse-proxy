#pragma once
#include "httprequestbuilder.hpp"
#include "request.hpp"
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
        http::HttpRequestBuilder builder_;
        asio::awaitable<http::HttpRequest> async_read_request();

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
        std::string current_header_field_;
        std::string current_header_value_;

        // TODO: implement this shit
        // processing::processingConfig config_;

        bool request_ready_ = false;
        std::optional<http::HttpRequest> current_request_;

    };


}

/*
namespace http
{
    static HttpMethod map_method(llhttp_method_t m) {
        switch (m) {
            case HTTP_GET:     return HttpMethod::GET;
            case HTTP_HEAD:    return HttpMethod::HEAD;
            case HTTP_POST:    return HttpMethod::POST;
            case HTTP_PUT:     return HttpMethod::PUT;
            case HTTP_DELETE:  return HttpMethod::DELETE;
            case HTTP_CONNECT: return HttpMethod::CONNECT;
            case HTTP_OPTIONS: return HttpMethod::OPTIONS;
            case HTTP_TRACE:   return HttpMethod::TRACE;
            case HTTP_PATCH:   return HttpMethod::PATCH;
            default:           return HttpMethod::UNKNOWN;
        }
    }
}
*/