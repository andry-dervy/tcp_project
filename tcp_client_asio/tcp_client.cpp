#include "tcp_client.h"

namespace tcp_client {

tcp_client::tcp_client()
    : thread_ptr_(nullptr)
{
    start();
}

tcp_client::~tcp_client()
{
    stop();
    if(thread_ptr_->joinable())
    {
        thread_ptr_->join();
    }
    std::cout << "Stop thread ios." << std::endl;
}

const std::shared_ptr<logger::logger>& tcp_client::logger() const
{
    return logger_;
}

void tcp_client::set_logger(const std::shared_ptr<logger::logger>& logger)
{
    logger_ = logger;
}

void tcp_client::start()
{
    std::cout << "Start thread ios." << std::endl;

    if(!thread_ptr_)
    {
        thread_ptr_ = std::make_unique<std::thread>([](){
            get_ios().run();
        });
    }
}

void tcp_client::stop()
{
    if(!get_ios().stopped()) get_ios().stop();
}

} // namespace tcp_client
