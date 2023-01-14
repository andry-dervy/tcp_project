#include "logger.h"

namespace logger {

void logger::log_(std::string& msg)
{
    std::lock_guard<std::mutex> lock(mtx_);
    ostream_ << msg << std::endl;
}

void logger::log(std::string&& msg)
{
    log_(msg);
    //    std::lock_guard<std::mutex> lock(mtx_);
//    ostream_ << msg << std::endl;
}
void logger::log(std::string& msg)
{
    log_(msg);
    //    std::lock_guard<std::mutex> lock(mtx_);
//    ostream_ << msg << std::endl;
}

logger::logger(std::ostream &ostream)
    : ostream_(ostream)
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

} // namespace logger
