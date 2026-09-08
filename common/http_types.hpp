#pragma once

#include <string_view>

namespace http
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
        TRACE,

        UNKNOWN
    };

    struct HttpVersion
    {
        int32_t version_major;
        int32_t version_minor_;
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
            default:
        }
        return "";
    }

    constexpr std::string_view toString(HttpVersion v)
    {
        switch (v.version_major * 10 + v.version_minor_)
        {
            case 10: return "HTTP/1.0";
            case 11: return "HTTP/1.1";
            case 20: return "HTTP/2";
            case 30: return "HTTP/3";
            default: return "";
        }
    }
}


