#pragma once

#include "http_types.hpp"
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

namespace http
{
    struct HttpResponse
    {
        uint16_t status_code;
        std::string status_message;
        HttpVersion version;
        std::vector<std::pair<std::string, std::string>> headers;
        std::string body;
    };
}
