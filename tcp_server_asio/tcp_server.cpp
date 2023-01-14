#include "tcp_server.h"

namespace tcp_server {

std::string make_daytime_string()
{
    using namespace std; // For time_t, time and ctime;
    time_t now = time(0);
    return ctime(&now);
}

tcp_connection::~tcp_connection()
{
    if(socket_.is_open())
        stop();
}

void tcp_connection::start()
{
    do_read();
}

void tcp_connection::stop()
{
    socket_.close();
}

void tcp_connection::do_read()
{
    auto self(shared_from_this());
    socket_.async_read_some(boost::asio::buffer(&data_in_[0], max_length),
                            [this, self](std::error_code ec, std::size_t length)
    {
        if (!ec)
        {
            auto it = protocols_.find(current_type_protocol_);
            if(it == protocols_.end())
                return;

            std::vector<char> data(length);
            std::memcpy(&data[0],&data_in_[0], length);

            it->second->recieve(self, std::move(data));

            do_read();
        }
    });
}

void tcp_connection::do_write(std::size_t length)
{
    auto self(shared_from_this());
    boost::asio::async_write(socket_, boost::asio::buffer(&data_out_[0], length),
                      [](boost::system::error_code ec, std::size_t /*length*/)
    {
        if (ec)
        {
            std::cout << "Error: " << ec << std::endl;
        }
    });
}

void tcp_connection::send(std::vector<char> out)
{
    size_t length = out.size() <= data_out_.size() ? out.size() : data_out_.size();
    std::memcpy(&data_out_[0], &out[0], length);
    do_write(length);
}

void tcp_connection::send(std::string out)
{
    size_t length = out.size() <= data_out_.size() ? out.size() : data_out_.size();
    std::memcpy(&data_out_[0], &out[0], length);
    do_write(length);
}

//---------------------------------------------------------------

tcp_server::tcp_server(std::shared_ptr<logger::logger> log, io_context_t& io_context)
    : logger::logable(log)
    , io_context_(io_context)
    , acceptor_(io_context,
                endpoint_t(boost::asio::ip::make_address("127.0.0.1"),
                                        13000))
{
    start_accept();
}

tcp_server::tcp_server(std::shared_ptr<logger::logger> log, io_context_t& io_context, std::string address, int port)
    : logger::logable(log)
    , io_context_(io_context)
    , acceptor_(io_context,
                endpoint_t(boost::asio::ip::make_address(address),
                                        port))
{
    std::cout << "Start server: " << address << " / " << port << std::endl;
    start_accept();
}

void tcp_server::add_protocol(std::unique_ptr<protocol::protocol>&& protocol)
{
    protocols_.insert({protocol->get_type_protocol(),std::move(protocol)});
}

bool tcp_server::erase_connection(std::shared_ptr<tcp_connection> connection)
{
    auto it = std::find_if(connections_.begin(),connections_.end(),
                 [&connection](std::shared_ptr<tcp_connection> c){
        return connection == c;
    });

    if(it != connections_.end())
    {
        connections_.erase(it);
        return true;
    }
    return false;
}

void tcp_server::start_accept()
{
    acceptor_.async_accept(
        [this](std::error_code ec, socket_t socket)
        {
            if (!ec)
            {
                auto connection = std::make_shared<tcp_connection>(std::move(socket),
                                                                   protocols_);
                connection->start();
                connections_.emplace_back(connection);
            }

            start_accept();
        });
}

} // namespace tcp_server
