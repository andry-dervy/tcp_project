#include "user_client.h"

namespace user {

user_client::user_client(std::shared_ptr<logger::logger>& logger)
    : logger::logable(logger)
{}

user_client::~user_client()
{
    if(thread)
    {
        tcp_client::tcp_client::stop();
    }
    //    out_ << " ui_cli_client destruction." << std::endl;
}

void user_client::add_request(std::shared_ptr<request_client::request> request_ptr)
{
    requests_.insert(request_ptr);
    request_ptr->execute();
}

void tcp_client_start()
{
    tcp_client::tcp_client::start();
}

void user_client::start_thread_tcp_client()
{
    std::thread thread(tcp_client_start);
    thread.detach();
}

} // namespace ui
