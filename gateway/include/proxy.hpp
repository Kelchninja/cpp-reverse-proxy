#pragma once
#include "request.hpp"
#include "response.hpp"
#include <asio/ip/tcp.hpp>

namespace gateway
{
    void prepare_upstream_request(http::HttpRequest&, const asio::ip::tcp::endpoint&);
    void prepare_downstream_response(http::HttpResponse&);
}
