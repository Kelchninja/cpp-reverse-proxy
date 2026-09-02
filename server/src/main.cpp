#include "config.hpp"
#include "server.hpp"
#include <asio.hpp>
#include <filesystem>


int main() 
{
    using asio::ip::tcp;

    // configuration loading
    std::filesystem::path config_path{"config.json"};
    config::ServerConfig server_config{config::ServerConfig::load(config_path)};

    // setting up server IO
    asio::io_context io_ctx;
    tcp::acceptor acceptor{io_ctx, tcp::endpoint{tcp::v4(), 12345}};

    // creating server
    server::Server server{io_ctx, std::move(acceptor), server_config};
    server.init();
    server.run();


    // shutdown and cleanup
    
}