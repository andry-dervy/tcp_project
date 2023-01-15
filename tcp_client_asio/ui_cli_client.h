#pragma once

#include <iostream>
#include <memory>
#include <thread>
#include "../logger/logger.h"
#include "tcp_client.h"

namespace ui {

class request_callback
{
public:
    virtual ~request_callback();

    virtual tcp_client::connection_ptr&& on_send(tcp_client::connection_ptr&& c) const = 0;
    virtual tcp_client::connection_ptr&& on_read(tcp_client::connection_ptr&& c) const = 0;
};

class universe_request_callback: public request_callback
{
public:
    ~universe_request_callback() override;
    universe_request_callback(std::ostream& out) : out_(out) {}
    tcp_client::connection_ptr&& on_send(tcp_client::connection_ptr&& c) const override;
    tcp_client::connection_ptr&& on_read(tcp_client::connection_ptr&& c) const override;
private:
    std::ostream& out_;
};

class ui_client
{
public:
    virtual ~ui_client();

    void set_tcp_client(std::unique_ptr<tcp_client::tcp_client>&& client) {client_ = std::move(client);}

    virtual void start() = 0;
protected:
    std::unique_ptr<tcp_client::tcp_client> client_;
};

class ui_cli_client: public ui_client, public logger::logable
{
public:
    ui_cli_client(std::shared_ptr<logger::logger> logger, std::istream& in, std::ostream& out);
    ~ui_cli_client() override;
    void start() override;
private:
    std::istream& in_;
    std::ostream& out_;
    std::unique_ptr<std::thread> thread;
};

} // namespace ui

