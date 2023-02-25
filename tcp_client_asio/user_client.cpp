#include "user_client.h"

namespace user {

user_client::user_client(tcp_client::tcp_client& client)
    : logger::logable(client.logger())
    , client_ref_(client)
{
}

user_client::user_client(tcp_client::tcp_client& client, std::string address, int port)
    : request_client::addressable(address, port)
    , logger::logable(client.logger())
    , client_ref_(client)
{
}

user_client::~user_client()
{
}

void user_client::add_request(std::shared_ptr<request_client::request> request_ptr)
{
    request_ptr->set_address(address());
    request_ptr->set_port(port());
    requests_.insert(request_ptr);
    request_ptr->execute();
}


} // namespace ui
