#include "tcp_client.h"

namespace tcp_client {

tcp_client::tcp_client(std::shared_ptr<logger::logger> logger, io_context_t& io_context)
    : logger::logable(logger), io_context_(io_context), socket_(io_context)
{

}

void tcp_client::connect()
{
    endpoint_t endpoint(
        boost::asio::ip::address::from_string(address_), port_);

    socket_.connect(endpoint);
}

void tcp_client::connect(std::string address, int port)
{
    address_ = address;
    port_ = port;

    connect();
}

void tcp_client::write(std::string req)
{
    socket_.write_some(boost::asio::buffer(req));
}

std::string tcp_client::read(boost::system::error_code& error)
{
    std::array<char, 128> buf;
    size_t len = socket_.read_some(boost::asio::buffer(buf), error);
    if(len != 0) return std::string(buf.begin(),buf.begin()+len);
    return std::string();
}

} // namespace tcp_client
