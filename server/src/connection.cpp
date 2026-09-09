#include "connection.hpp"
#include "llhttp.h"
#include <asio/co_spawn.hpp>
#include <asio/detached.hpp>
#include <asio/ip/tcp.hpp>

namespace
{
    inline server::Connection* getInstancePtr(llhttp_t *p)
    {
        return static_cast<server::Connection*>(p->data);
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
    }

    asio::awaitable<http::HttpRequest> Connection::async_read_request()
    {
        if (llhttp_get_errno(&parser_) == HPE_PAUSED)
        {
            llhttp_resume(&parser_);
        }

        while (true) {
            std::error_code ec;
            size_t n = co_await socket_.async_read_some(
                buf_.prepare(8192),
                asio::redirect_error(asio::use_awaitable, ec));

            if (ec) throw std::runtime_error("connection closed");

            buf_.commit(n);
            auto data = asio::buffer_cast<const char*>(buf_.data());
            auto err = llhttp_execute(&parser_, data, n);
            buf_.consume(n);

            if (err != HPE_OK && err != HPE_PAUSED)
                throw std::runtime_error("parse error");

            if (request_ready_) {
                request_ready_ = false;
                co_return std::move(*current_request_);
            }
        }
    }

    int Connection::on_url(llhttp_t *parser, const char *at, size_t len)
    {
        getInstancePtr(parser)->builder_.append_url(at, len);
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
        self->builder_.add_header(
            std::move(self->current_header_field_), 
            std::move(self->current_header_value_)
        );

        self->current_header_field_.clear();
        self->current_header_value_.clear();

        return 0;
    }

    int Connection::on_body(llhttp_t *parser, const char *at, size_t len)
    {
        getInstancePtr(parser)->builder_.append_body(at, len);
        return 0;
    }

    int Connection::on_message_complete(llhttp_t* parser)
    {
        auto* self{getInstancePtr(parser)};
        auto method{static_cast<http::HttpMethod>(parser->method)};
        self->builder_.set_method(method);
        self->builder_.set_version({parser->http_major, parser->http_minor});

        self->current_request_.emplace(std::move(self->builder_).build());
        self->request_ready_ = true;
        return HPE_PAUSED; // pause parser until request is collected
    }
}