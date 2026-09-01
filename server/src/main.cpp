#include <asio.hpp>


int main() 
{
    using asio::ip::tcp;
    
    asio::io_context io_ctx;
    tcp::acceptor acceptor{io_ctx, tcp::endpoint{tcp::v4(), 12345}};

}