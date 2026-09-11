#pragma once
#include "http_types.hpp"
#include "request.hpp"
#include "response.hpp"
#include <string>


namespace http
{
    class HttpSerializer
    {
        public:
        std::string serialize_request(const HttpRequest& req) const;
        std::string serialize_response(const HttpResponse& res) const;
    };
}


