#pragma once

#include <iostream>
#include "boost/asio.hpp"
#include "../logger/logger.h"

namespace tcp_client {

using socket_t = boost::asio::ip::tcp::socket;
using acceptor_t = boost::asio::ip::tcp::acceptor;
using io_context_t = boost::asio::io_context;
using endpoint_t = boost::asio::ip::tcp::endpoint;

class tcp_client: public logger::logable
{
public:
    tcp_client(std::shared_ptr<logger::logger> logger, io_context_t& io_context);

    void connect(std::string address, int port);
    void connect();

    void write(std::string req);
    std::string read(boost::system::error_code& error);

    void set_address(std::string address) {address_ = address;}
    void set_port(int port) {port_ = port;}

private:
    io_context_t& io_context_;
    socket_t socket_;

    std::string address_;
    int port_;
};

} // namespace tcp_client
