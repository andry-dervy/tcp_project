#pragma once

#include <iostream>
#include <deque>
#include <map>
#include <memory>
#include "boost/asio.hpp"
#include "protocol.h"
#include "../logger/logger.h"

namespace tcp_server {

using socket_t = boost::asio::ip::tcp::socket;
using acceptor_t = boost::asio::ip::tcp::acceptor;
using io_context_t = boost::asio::io_context;
using endpoint_t = boost::asio::ip::tcp::endpoint;

class tcp_connection
    : public std::enable_shared_from_this<tcp_connection>
{
public:
    tcp_connection(socket_t socket,
                   std::map<protocol::eTypeProtocol,std::shared_ptr<protocol::protocol>>& protocols,
                   protocol::eTypeProtocol type_protocol = protocol::eTypeProtocol::CommandLine)
        : socket_(std::move(socket))
        , protocols_(protocols)
        , current_type_protocol_(type_protocol)
        , data_out_(max_length)
        , data_in_(max_length)
    {}
    tcp_connection(socket_t socket,
                   std::map<protocol::eTypeProtocol,std::shared_ptr<protocol::protocol>>& protocols)
        : socket_(std::move(socket))
        , protocols_(protocols)
        , current_type_protocol_(protocol::eTypeProtocol::CommandLine)
        , data_out_(max_length)
        , data_in_(max_length)
    {}
    ~tcp_connection();

    socket_t& socket() {return socket_;}

    void start();
    void stop();

private:
    void do_read();
    void do_write(std::size_t length);
public:
    void send(std::vector<char> out);
    void send(std::string out);

private:
    socket_t socket_;
    std::map<protocol::eTypeProtocol,std::shared_ptr<protocol::protocol>> protocols_;
    protocol::eTypeProtocol current_type_protocol_;

    enum { max_length = 1024 };
//    char data_[max_length];
    std::vector<char> data_out_;
    std::vector<char> data_in_;
    std::string message_;
};

class tcp_server: public logger::logable
{
public:
    tcp_server(std::shared_ptr<logger::logger> logger, io_context_t& io_context);
    tcp_server(std::shared_ptr<logger::logger> logger, io_context_t& io_context, std::string address, int port);

    void add_protocol(std::shared_ptr<protocol::protocol>&& protocol);

    bool erase_connection(std::shared_ptr<tcp_connection> connection);
private:
    void start_accept();

    io_context_t& io_context_;
    acceptor_t acceptor_;

    std::deque<std::shared_ptr<tcp_connection>> connections_;
    std::map<protocol::eTypeProtocol,std::shared_ptr<protocol::protocol>> protocols_;
    protocol::eTypeProtocol type_protocol_;
};

} // namespace tcp_server
