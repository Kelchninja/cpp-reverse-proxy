#pragma once
#include "export.hpp"
#include "request.hpp"

namespace gateway
{
    BACKEND_API void send_to_backend(http::HttpRequest req);
}


