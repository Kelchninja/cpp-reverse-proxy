#include "gateway.hpp"
#include "request.hpp"
#include "response.hpp"
#include <asio.hpp>

namespace gateway
{

    asio::awaitable<http::HttpResponse> send_to_backend(const http::HttpRequest& req)
    {
        using asio::ip::tcp;

        tcp::socket backend_socket{co_await asio::this_coro::executor};

        co_await backend_socket.async_connect(flask_endpoint, asio::use_awaitable);
        std::string raw_request = serialize_request(req);
        co_await asio::async_write(backend_socket, asio::buffer(raw_request), asio::use_awaitable);

        http::HttpResponse response = co_await read_and_parse_response(backend_socket);

        co_return response;
    }

    asio::
}
