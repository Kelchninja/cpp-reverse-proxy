
#include "connection.hpp"
#include "response.hpp"
#include "llhttp.h"
#include <algorithm>
#include <asio/awaitable.hpp>
#include <asio/ip/tcp.hpp>
#include <cctype>
#include <stdexcept>

namespace
{
    inline server::UpstreamConnection* getInstancePtr(llhttp_t *p)
    {
        return static_cast<server::UpstreamConnection*>(p->data);
    }

    inline void to_lower(std::string& s)
    {
        std::transform(s.begin(), s.end(), s.begin(),
            [](unsigned char c) { return std::tolower(c); });
    }
}


namespace server
{
    UpstreamConnection::UpstreamConnection(asio::ip::tcp::socket upstream_socket): socket_{std::move(upstream_socket)}
    {
        llhttp_settings_init(&settings_);

        // callback mapping
        settings_.on_status = on_status;
        settings_.on_header_field = on_header_field;
        settings_.on_header_value = on_header_value;
        settings_.on_header_value_complete = on_header_value_complete;
        settings_.on_body = on_body;
        settings_.on_message_complete = on_message_complete;

        // init parser
        llhttp_init(&parser_, HTTP_RESPONSE, &settings_);
        parser_.data = this; // return route to this instance

        current_response_.emplace();
    }

    asio::awaitable<void> UpstreamConnection::send_request(const http::HttpRequest& req)
    {
        std::string raw_request{serializer_.serialize_request(req)};
        co_await asio::async_write(socket_, asio::buffer(raw_request), asio::use_awaitable);
    }

    asio::awaitable<http::HttpResponse> UpstreamConnection::async_read_response()
    {
        if (llhttp_get_errno(&parser_) == HPE_PAUSED)
        {
            llhttp_resume(&parser_);
        }

        while (true) {
            bool eof{false};

            if (buf_.size() == 0) {
                std::error_code ec;
                size_t n = co_await socket_.async_read_some(
                    buf_.prepare(8192),
                    asio::redirect_error(asio::use_awaitable, ec));

                if (ec == asio::error::eof) {
                    eof = true;
                } else if (ec) {
                    throw std::runtime_error("upstream connection closed");
                } else {
                    buf_.commit(n);
                }
            }

            llhttp_errno_t err{HPE_OK};
            if (eof) {
                err = llhttp_finish(&parser_);
            } else {
                auto data = asio::buffer_cast<const char*>(buf_.data());
                auto size = buf_.size();
                err = llhttp_execute(&parser_, data, size);

                size_t consumed{err == HPE_PAUSED
                    ? static_cast<size_t>(llhttp_get_error_pos(&parser_) - data)
                    : size};
                buf_.consume(consumed);
            }

            if (err != HPE_OK && err != HPE_PAUSED)
                throw std::runtime_error("parse error");

            if (response_ready_) {
                response_ready_ = false;
                http::HttpResponse response{std::move(*current_response_)};
                current_response_.emplace(); // fresh target for the next keep-alive response
                co_return response;
            }

            if (eof) throw std::runtime_error("upstream closed before response was complete");
        }
    }

    int UpstreamConnection::on_status(llhttp_t *parser, const char *at, size_t len)
    {
        getInstancePtr(parser)->current_response_->status_message.append(at, len);
        return 0;
    }

    int UpstreamConnection::on_header_field(llhttp_t *parser, const char *at, size_t len)
    {
        getInstancePtr(parser)->current_header_field_.append(at, len);
        return 0;
    }

    int UpstreamConnection::on_header_value(llhttp_t *parser, const char *at, size_t len)
    {
        getInstancePtr(parser)->current_header_value_.append(at, len);
        return 0;
    }

    int UpstreamConnection::on_header_value_complete(llhttp_t *parser)
    {
        auto* self{getInstancePtr(parser)};
        to_lower(self->current_header_field_);
        self->current_response_->headers.emplace_back(
            std::move(self->current_header_field_),
            std::move(self->current_header_value_)
        );

        self->current_header_field_.clear();
        self->current_header_value_.clear();

        return 0;
    }

    int UpstreamConnection::on_body(llhttp_t *parser, const char *at, size_t len)
    {
        getInstancePtr(parser)->current_response_->body.append(at, len);
        return 0;
    }

    int UpstreamConnection::on_message_complete(llhttp_t* parser)
    {
        auto* self{getInstancePtr(parser)};
        self->current_response_->status_code = static_cast<uint16_t>(parser->status_code);
        self->current_response_->version = {parser->http_major, parser->http_minor};

        self->response_ready_ = true;
        return HPE_PAUSED; // pause parser until response is collected
    }

}
