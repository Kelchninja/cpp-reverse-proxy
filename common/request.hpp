#pragma once

#include "http_types.hpp"
#include <string>
#include <utility>
#include <vector>

namespace http
{

    // --- domain structs / classes ---

    struct HttpRequest
    {
        HttpMethod method = HttpMethod::UNKNOWN;
        std::string path;
        HttpVersion version{1, 1};
        std::vector<std::pair<std::string, std::string>> headers;
        std::string body;
    };
}
