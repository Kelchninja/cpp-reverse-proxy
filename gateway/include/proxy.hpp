#pragma once
#include "request.hpp"
#include "response.hpp"
#include <asio/ip/tcp.hpp>

namespace gateway
{
    // Macht aus dem Client-Request den Request, der ans Backend geht:
    // Hop-by-hop-Header raus, X-Forwarded-*/Via rein, Content-Length aus dem Body.
    void prepare_upstream_request(http::HttpRequest&, const asio::ip::tcp::endpoint& client);

    // Gegenrichtung: Backend-Response so aufbereiten, dass sie an den Client kann.
    void prepare_downstream_response(http::HttpResponse&);
}
