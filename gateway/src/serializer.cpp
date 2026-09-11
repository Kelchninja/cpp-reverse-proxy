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

        req_string.append("\r\n");
        req_string.append(req.body);

        return req_string;
    }

    std::string HttpSerializer::serialize_response(const HttpResponse& res) const
    {
        std::string res_string;
        res_string.append(http::toString(res.version)).append(" ");
        res_string.append(std::to_string(res.status_code)).append(" ");
        res_string.append(res.status_message);
        res_string.append("\r\n");

        for (const auto& pair : res.headers)
        {
            res_string.append(pair.first).append(": ").append(pair.second);
            res_string.append("\r\n");
        }

        res_string.append("\r\n");
        res_string.append(res.body);

        return res_string;
    }

}

