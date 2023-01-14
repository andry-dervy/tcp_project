#pragma once

#include <iostream>
#include <memory>
#include "../logger/logger.h"
#include "tcp_client.h"

namespace ui {

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
    void start() override;
private:
    std::istream& in_;
    std::ostream& out_;

};

} // namespace ui

