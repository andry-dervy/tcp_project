#include "logger.h"
#include <chrono>
#include <algorithm>
#include <ctime>
#include <iomanip>


namespace logger {

void logger::log_(std::string& msg)
{
    std::lock_guard<std::mutex> lock(mtx_);
    auto now =  std::chrono::system_clock::now();
    const std::time_t t_c = std::chrono::system_clock::to_time_t(now);
    ostream_ << std::put_time(std::localtime(&t_c), "%F %T: ") << msg << std::endl << std::flush;
}

void logger::log(std::string&& msg)
{
    log_(msg);
}
void logger::log(std::string& msg)
{
    log_(msg);
}

logger::logger(std::ostream &ostream)
    : stream(ostream)
{}

void logger::operator()(std::string &msg)
{
    log_(msg);
}

void logger::operator()(std::string &&msg)
{
    log_(msg);
}

logable::~logable() {}

file::file(const std::string& file_name)
    : os_(std::ofstream(file_name, std::ios_base::out))
    , is_open(false)
{
    if(os_) is_open = true;
}

file::~file()
{
    if(os_) os_.close();
}

file_logger::file_logger(const std::string& file_name)
    : file(file_name), logger(os_)
{
}

} // namespace logger
