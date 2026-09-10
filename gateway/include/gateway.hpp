#pragma once
#include "request.hpp"
#include "response.hpp"
#include <asio/awaitable.hpp>

namespace gateway
{
    asio::awaitable<http::HttpResponse> send_to_backend(const http::HttpRequest& req);
}


