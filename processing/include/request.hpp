#include "export.hpp"
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace processing
{
    // --- enums ---

    enum class HttpMethod
    {
        GET,
        POST,
        PUT,
        DELETE,
        PATCH,
        HEAD,
        OPTIONS,
        CONNECT,
        TRACE
    };

    enum class HttpVersion
    {
        Http1_0,
        Http1_1,
        Http2,
        Http3
    };

    // --- helper functions ---

    constexpr std::string_view toString(HttpMethod m)
    {
        switch (m)
        {
            case HttpMethod::GET:     return "GET";
            case HttpMethod::POST:    return "POST";
            case HttpMethod::PUT:     return "PUT";
            case HttpMethod::DELETE:  return "DELETE";
            case HttpMethod::PATCH:   return "PATCH";
            case HttpMethod::HEAD:    return "HEAD";
            case HttpMethod::OPTIONS: return "OPTIONS";
            case HttpMethod::CONNECT: return "CONNECT";
            case HttpMethod::TRACE:   return "TRACE";
        }
        return "";
    }

    constexpr std::string_view toString(HttpVersion v)
    {
        switch (v)
        {
            case HttpVersion::Http1_0: return "HTTP/1.0";
            case HttpVersion::Http1_1: return "HTTP/1.1";
            case HttpVersion::Http2:   return "HTTP/2";
            case HttpVersion::Http3:   return "HTTP/3";
        }
        return "";
    }

    // --- domain structs / classes ---

    struct HttpRequest
    {
        const HttpMethod method;
        const std::string path;
        const HttpVersion version;
        const std::unordered_map<std::string, std::string> query_params;
        const std::unordered_map<std::string, std::string> headers;
        const std::vector<char> body;
    };
}