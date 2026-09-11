#include "gateway.hpp"
#include "connection.hpp"
#include "proxy.hpp"
#include "request.hpp"
#include "response.hpp"
#include <asio.hpp>
#include <memory>

namespace gateway
{

    Gateway::Gateway(const config::GatewayConfig& config)
    : config_{config},
      endpoint_{asio::ip::make_address_v4(config_.ipv4), config_.port} {}

    asio::awaitable<http::HttpResponse> Gateway::send_to_backend(http::HttpRequest req, asio::ip::tcp::endpoint client)
    {
        using asio::ip::tcp;

        prepare_upstream_request(req, client);

        tcp::socket backend_socket{co_await asio::this_coro::executor};

        co_await backend_socket.async_connect(endpoint_, asio::use_awaitable);

        auto upstream{std::make_shared<server::UpstreamConnection>(std::move(backend_socket))};
        co_await upstream->send_request(req);

        http::HttpResponse res{co_await upstream->async_read_response()};
        prepare_downstream_response(res);
        co_return res;
    }

}
