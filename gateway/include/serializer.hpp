#include "http_types.hpp"
#include "request.hpp"
#include <string>


namespace http
{
    class HttpSerializer
    {
        public:
        std::string serialize_request(const HttpRequest& req) const;
    };
}


