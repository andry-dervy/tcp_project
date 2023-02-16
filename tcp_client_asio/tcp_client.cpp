#include "tcp_client.h"

namespace tcp_client {

tcp_client::tcp_client(std::shared_ptr<logger::logger> logger)
    : logger::logable(logger)
{}

tcp_client::~tcp_client()
{
    stop();
}

void tcp_client::start()
{
    std::cout << "Start thread ios." << std::endl;
    get_ios().run();
    std::cout << "Stop thread ios." << std::endl;
}

void tcp_client::stop()
{
    if(!get_ios().stopped()) get_ios().stop();
}

} // namespace tcp_client
