#pragma once

#include "http_types.hpp"
#include "request.hpp"

#include <cstddef>
#include <string>
#include <utility>
#include <vector>

namespace http
{

    class HttpRequestBuilder
    {
        public:
        void set_method(HttpMethod method)
        {
            method_ = method;
        }

        void append_url(const char *data, size_t len)
        {
            url_.append(data, len);
        }

        void set_version(HttpVersion version)
        {
            version_ = version;
        }

        void add_header(std::string field, std::string value)
        {
            std::transform(field.begin(), field.end(), field.begin(),
                [](unsigned char c) { return std::tolower(c); });
            headers_.emplace_back(std::move(field), std::move(value));
        }

        void append_body(const char* data, std::size_t len)
        {
            body_.append(data, len);
        }

        HttpRequest build() &&
        {
            return HttpRequest{
                method_,
                std::move(url_),
                version_,
                std::move(headers_),
                std::move(body_)
            };
        }

        private:
        HttpMethod method_ = HttpMethod::UNKNOWN;
        std::string url_;
        HttpVersion version_{1, 1};
        std::vector<std::pair<std::string, std::string>> headers_;
        std::string body_;
    };

} // namespace http