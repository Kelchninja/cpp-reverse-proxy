#include "serializer.hpp"
#include "http_types.hpp"
#include <string>


namespace http
{
    std::string HttpSerializer::serialize_request(const HttpRequest& req) const
    {
        std::string req_string;
        req_string.append(http::toString(req.method)).append(" ");
        req_string.append(req.path).append(" ");
        req_string.append(http::toString(req.version));
        req_string.append("\r\n");

        for (const auto& pair : req.headers)
        {
            req_string.append(pair.first).append(": ").append(pair.second);
            req_string.append("\r\n");
        }

        req_string.append("\r\n"); // Leerzeile: trennt Header von Body
        req_string.append(req.body);

        return req_string;
    }

}

