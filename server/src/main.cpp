#include "config.hpp"
#include "gateway.hpp"
#include "server.hpp"
#include <asio.hpp>
#include <asio/ip/address_v4.hpp>
#include <filesystem>


int main() 
{
    using asio::ip::tcp;

    // configuration loading
    std::filesystem::path config_path{"config.json"};
    config::ServerConfig server_config = config::ServerConfig::load(config_path);
    config::GatewayConfig gateway_config = config::GatewayConfig::load(config_path);

    std::println("config loaded - running cores: {} (auto-detect: {})",
        server_config.available_cores, server_config.auto_detect_cores);
    std::println("gateway config loaded - backend at {}:{}",
        gateway_config.ipv4, gateway_config.port);

    // initialize gateway
    gateway::Gateway gateway{gateway_config};

    // setting up server IO
    asio::io_context io;
    tcp::acceptor acceptor{io, 
        tcp::endpoint{asio::ip::make_address_v4(server_config.ipv4), 
            server_config.port}};

    // creating server
    server::Server server{io, std::move(acceptor), gateway, server_config};
    server.init();

    std::println("server initialized - listening on {}:{}", 
        server_config.ipv4, server_config.port);
    
    server.run();

    std::println("server running.");

    // shutdown and cleanup
    asio::signal_set signals{io, SIGINT, SIGTERM};
    signals.async_wait([&server](const asio::error_code&, int)
    {
        server.stop();
        std::println("server stopped.");
    });
    
    // keeping server alive (no busy waiting)
    server.wait();

    return 0;
}