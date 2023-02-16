#pragma once

#include "tcp_client.h"
#include "request.h"

namespace user {

class user_client: public logger::logable
{
public:
    user_client(std::shared_ptr<logger::logger>& logger);
    virtual ~user_client();

    void set_tcp_client(std::unique_ptr<tcp_client::tcp_client>&& client) {client_ = std::move(client);}

    virtual void start() = 0;
    virtual void stop() = 0;

    template<typename T, typename... Args>
    std::shared_ptr<T> make_request(Args&... args) {
        return std::make_shared<T>(*client_, args...);
    }

    void add_request(std::shared_ptr<request_client::request> request_ptr);

protected:
    void start_thread_tcp_client();

protected:
    std::unique_ptr<tcp_client::tcp_client> client_;
    std::set<std::shared_ptr<request_client::request>> requests_;

private:
    std::unique_ptr<std::thread> thread;
};

} // namespace ui
