#pragma once

#include <iostream>
#include <memory>
#include "../logger/logger.h"
#include "tcp_client.h"

namespace request_client {

enum class eStateRequest
{
    NotReadyToExecute,
    ReadyToExecute,
    ReadyToSend,
    WaitRead,
    Done,
    TimeOut
};

class addressable
{
public:
    addressable(){}
    addressable(std::string address, int port)
        : address_(address), port_(port) {}
    virtual ~addressable(){}

    const std::string &address() const;
    void set_address(const std::string &address);

    int port() const;
    void set_port(int port);

private:
    std::string address_;
    int port_;
};

class request : public addressable, public std::enable_shared_from_this<request>
{
public:
    request(tcp_client::tcp_client& client)
        : client_(client),
          log_(*client_.logger()),
          state_(eStateRequest::NotReadyToExecute) {}
    virtual ~request();

    eStateRequest get_state() const {return state_;}
    void set_data(std::vector<char>&& data) {data_ = data;}
    void execute();

    virtual void on_send() const = 0;
    virtual void on_read(std::vector<char>&& data) const = 0;
    virtual void on_timeout() const = 0;

private:
    tcp_client::tcp_client& client_;
    std::shared_ptr<boost::asio::deadline_timer> timer_;
    tcp_client::connection_ptr connection_;

    mutable std::mutex mtx_state;

protected:
    std::vector<char> data_;
    logger::logger& log_;
    eStateRequest state_;

    void set_state(eStateRequest state);
};

class universe_request: public request
{
public:
    universe_request(tcp_client::tcp_client& client, std::ostream& out)
        : request(client), out_(out) {
        state_ = eStateRequest::ReadyToExecute;
    }
    ~universe_request() override;

    void on_send() const override;
    void on_read(std::vector<char>&& data) const override;
    void on_timeout() const override;
private:
    std::ostream& out_;
};

} // namespace request
