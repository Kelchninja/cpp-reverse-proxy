#pragma once

#include "http_types.hpp"
#include <utility>
#include <vector>

namespace http
{

    // --- domain structs / classes ---

    struct HttpRequest
    {
        const HttpMethod method;
        const std::string path;
        const HttpVersion version;
        const std::vector<std::pair<std::string, std::string>> query_params;
        const std::vector<std::pair<std::string, std::string>> headers;
        const std::string body;
    };
}