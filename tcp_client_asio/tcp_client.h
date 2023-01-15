#pragma once

#include <iostream>
#include "boost/asio.hpp"
#include <boost/core/noncopyable.hpp>
#include "../logger/logger.h"

namespace tcp_client {

using socket_t = boost::asio::ip::tcp::socket;
using acceptor_t = boost::asio::ip::tcp::acceptor;
using io_context_t = boost::asio::io_context;
using endpoint_t = boost::asio::ip::tcp::endpoint;

struct connection_with_data: boost::noncopyable {
    boost::asio::ip::tcp::socket socket;
    std::vector<char> data;

    explicit connection_with_data(io_context_t& ios)
        : socket(ios)
    {}
    void shutdown()
    {
        if (!socket.is_open()) return;
        boost::system::error_code ignore;
        socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both,
                        ignore);
        socket.close(ignore);
    }
    ~connection_with_data() {
        shutdown();
    }
};

typedef std::unique_ptr<connection_with_data> connection_ptr;

template <class T>
struct task_wrapped_with_connection {
private:
    connection_ptr c_;
    T task_unwrapped_;
public:
    explicit task_wrapped_with_connection(connection_ptr&& c, const T& f)
        : c_(std::move(c))
        , task_unwrapped_(f)
    {}
    void operator()(const boost::system::error_code& error,
                    std::size_t bytes_count)
    {
        c_->data.resize(bytes_count);
        task_unwrapped_(std::move(c_), error);
    }
};

class tcp_client: public logger::logable
{
public:
    tcp_client(std::shared_ptr<logger::logger> logger);

    static io_context_t& get_ios() {
        static io_context_t ios;
        static boost::asio::io_service::work work(ios);
        return ios;
    }

    static connection_ptr create_connection(
        std::string address,
        unsigned short port)
    {
        connection_ptr c( new connection_with_data(get_ios()) );
        c->socket.connect(boost::asio::ip::tcp::endpoint(
            boost::asio::ip::address_v4::from_string(address),
            port));
        return c;
    }

    connection_ptr create_connection()
    {
        return create_connection(address_, port_);
    }

    template <class Functor>
    void async_write_data(connection_ptr&& c, const Functor& f) {
        boost::asio::ip::tcp::socket& s = c->socket;
        std::vector<char>& d = c->data;
        boost::asio::async_write(
            s,
            boost::asio::buffer(d),
            task_wrapped_with_connection<Functor>(std::move(c), f)
        );
    }

    template <class HandleRequest>
    void request(std::vector<char>&& data, const HandleRequest& fRequest)
    {
        auto c = create_connection();

        c->data = data;
        async_write_data(std::move(c),[=](connection_ptr&& c, const boost::system::error_code& err){
            if(err)
            {
                log("Error on sending data: " + err.message() + '\n');
                return;
            }
            auto&& c_ = fRequest.on_send(std::move(c));
            async_read_data_at_least(std::move(c_),[=](connection_ptr&& c, const boost::system::error_code& err)
            {
                if (err && err != boost::asio::error::eof) {
                    log("Client error on receive: " + err.message() + '\n');
                    return;
                }

                fRequest.on_read(std::move(c));
            },
            1,
            1024);
        });
    }

    template <class Functor>
    void async_read_data(connection_ptr&& c, const Functor& f, std::size_t at_least_bytes)
    {
        c->data.resize(at_least_bytes);
        boost::asio::ip::tcp::socket& s = c->socket;
        std::vector<char>& d = c->data;
        char* p = (d.empty() ? 0 : &d[0]);
        boost::asio::async_read(
            s,
            boost::asio::buffer(p, d.size()),
            task_wrapped_with_connection<Functor>(std::move(c), f)
        );
    }

    template <class Functor>
    void async_read_data_at_least(connection_ptr&& c, const Functor& f, std::size_t at_least_bytes, std::size_t at_most)
    {
        std::vector<char>& d = c->data;
        d.resize(at_most);
        char* p = (at_most == 0 ? 0 : &d[0]);
        boost::asio::ip::tcp::socket& s = c->socket;
        boost::asio::async_read(
            s,
            boost::asio::buffer(p, at_most),
            boost::asio::transfer_at_least(at_least_bytes),
            task_wrapped_with_connection<Functor>(std::move(c), f)
        );
    }

    void set_address(std::string address) {address_ = address;}
    void set_port(int port) {port_ = port;}

    static void start();
    static void stop();

private:
    std::string address_;
    int port_;
};

} // namespace tcp_client
