#pragma once

#include "tcp_client.h"
#include "request.h"

namespace user {

class user_client: public request_client::addressable, public logger::logable
{
public:
    user_client(tcp_client::tcp_client& client);
    user_client(tcp_client::tcp_client& client, std::string address, int port);
    virtual ~user_client();

    template<typename T, typename... Args>
    std::shared_ptr<T> make_request(Args&... args) {
        return std::make_shared<T>(client_ref_, args...);
    }

    void add_request(std::shared_ptr<request_client::request> request_ptr);

protected:
    tcp_client::tcp_client& client_ref_;
    std::set<std::shared_ptr<request_client::request>> requests_;
};

} // namespace ui
