#pragma once
#include "http_types.hpp"
#include "request.hpp"
#include <string>
#include <vector>

namespace http
{
    class HttpRequestBuilder
    {
        public:
        void setMethod(HttpMethod method) 
        { 
            method_ = method; 
        }
        void setUrl(std::string u) 
        { 
            url_ = std::move(u); 
        }
        void set_version(HttpVersion version)
        { 
            version_ = version; 
        }
        void add_header(std::string field, std::string value) 
        {
            headers_.emplace_back(std::move(field), std::move(value));
        }
        void add_query_param(std::string key, std::string value)
        {
            query_params_.emplace_back(std::move(key), std::move(value));
        }
        void append_body(const char* data, std::size_t len)
        {
            body_.append(data, len);
        }

        HttpRequest build()&& 
        {
            return HttpRequest {
                method_, 
                std::move(url_), 
                version_, 
                std::move(query_params_), 
                std::move(headers_), 
                std::move(body_)
            };
        }

        private:
        HttpMethod method_ = HttpMethod::UNKNOWN;
        std::string url_;
        HttpVersion version_;
        std::vector<std::pair<std::string, std::string>> headers_;
        std::vector<std::pair<std::string, std::string>> query_params_;
        std::string body_;
    };

}