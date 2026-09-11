#include "connection.hpp"
#include "llhttp.h"
#include <algorithm>
#include <asio/co_spawn.hpp>
#include <asio/detached.hpp>
#include <asio/ip/tcp.hpp>
#include <cctype>

namespace
{
    inline server::Connection* getInstancePtr(llhttp_t *p)
    {
        return static_cast<server::Connection*>(p->data);
    }

    inline void to_lower(std::string& s)
    {
        std::transform(s.begin(), s.end(), s.begin(),
            [](unsigned char c) { return std::tolower(c); });
    }

    http::HttpMethod map_method(llhttp_method_t m)
    {
        switch (m) {
            case HTTP_GET:     return http::HttpMethod::GET;
            case HTTP_HEAD:    return http::HttpMethod::HEAD;
            case HTTP_POST:    return http::HttpMethod::POST;
            case HTTP_PUT:     return http::HttpMethod::PUT;
            case HTTP_DELETE:  return http::HttpMethod::DELETE;
            case HTTP_CONNECT: return http::HttpMethod::CONNECT;
            case HTTP_OPTIONS: return http::HttpMethod::OPTIONS;
            case HTTP_TRACE:   return http::HttpMethod::TRACE;
            case HTTP_PATCH:   return http::HttpMethod::PATCH;
            default:           return http::HttpMethod::UNKNOWN;
        }
    }
}


namespace server
{
    Connection::Connection(asio::ip::tcp::socket client_socket): socket_{std::move(client_socket)}
    {
        llhttp_settings_init(&settings_);

        // callback mapping
        settings_.on_url = on_url;
        settings_.on_header_field = on_header_field;
        settings_.on_header_value = on_header_value;
        settings_.on_header_value_complete = on_header_value_complete;
        settings_.on_body = on_body;
        settings_.on_message_complete = on_message_complete;

        // init parser
        llhttp_init(&parser_, HTTP_REQUEST, &settings_);
        parser_.data = this; // return route to this instance

        current_request_.emplace();
    }

    asio::awaitable<http::HttpRequest> Connection::async_read_request()
    {
        if (llhttp_get_errno(&parser_) == HPE_PAUSED)
        {
            llhttp_resume(&parser_);
        }

        while (true) {
            if (buf_.size() == 0) {
                std::error_code ec;
                size_t n = co_await socket_.async_read_some(
                    buf_.prepare(8192),
                    asio::redirect_error(asio::use_awaitable, ec));

                if (ec) throw std::runtime_error("connection closed");
                buf_.commit(n);
            }

            auto data = asio::buffer_cast<const char*>(buf_.data());
            auto size = buf_.size();
            auto err = llhttp_execute(&parser_, data, size);

            size_t consumed{err == HPE_PAUSED
                ? static_cast<size_t>(llhttp_get_error_pos(&parser_) - data)
                : size};
            buf_.consume(consumed);

            if (err != HPE_OK && err != HPE_PAUSED)
                throw std::runtime_error("parse error");

            if (request_ready_) {
                request_ready_ = false;
                http::HttpRequest request{std::move(*current_request_)};
                current_request_.emplace(); // fresh target for the next keep-alive request
                co_return request;
            }
        }
    }

    asio::awaitable<void> Connection::send_response(const http::HttpResponse& res)
    {
        std::string raw_response{serializer_.serialize_response(res)};
        co_await asio::async_write(socket_, asio::buffer(raw_response), asio::use_awaitable);
    }

    asio::ip::tcp::endpoint Connection::remote_endpoint() const
    {
        return socket_.remote_endpoint();
    }

    int Connection::on_url(llhttp_t *parser, const char *at, size_t len)
    {
        getInstancePtr(parser)->current_request_->path.append(at, len);
        return 0;
    }

    int Connection::on_header_field(llhttp_t *parser, const char *at, size_t len)
    {
        getInstancePtr(parser)->current_header_field_.append(at, len);
        return 0;
    }

    int Connection::on_header_value(llhttp_t *parser, const char *at, size_t len)
    {
        getInstancePtr(parser)->current_header_value_.append(at, len);
        return 0;
    }

    int Connection::on_header_value_complete(llhttp_t *parser)
    {
        auto* self{getInstancePtr(parser)};
        to_lower(self->current_header_field_);
        self->current_request_->headers.emplace_back(
            std::move(self->current_header_field_),
            std::move(self->current_header_value_)
        );

        self->current_header_field_.clear();
        self->current_header_value_.clear();

        return 0;
    }

    int Connection::on_body(llhttp_t *parser, const char *at, size_t len)
    {
        getInstancePtr(parser)->current_request_->body.append(at, len);
        return 0;
    }

    int Connection::on_message_complete(llhttp_t* parser)
    {
        auto* self{getInstancePtr(parser)};
        self->current_request_->method = map_method(static_cast<llhttp_method_t>(parser->method));
        self->current_request_->version = {parser->http_major, parser->http_minor};

        self->request_ready_ = true;
        return HPE_PAUSED; // pause parser until request is collected
    }

}
