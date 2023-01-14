#ifndef LOGGER_H
#define LOGGER_H

#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <sstream>
#include <memory>
#include <mutex>

namespace logger {

class logger
{
public:
    logger(std::ostream &ostream);
    ~logger() {}

    void operator()(std::string &msg);
    void operator()(std::string &&msg);
    void log(std::string &msg);
    void log(std::string &&msg);

private:
    std::mutex mtx_;
    std::ostream &ostream_;
private:
    void log_(std::string &msg);
};

class logable
{
public:
    logable(std::shared_ptr<logger> logger)
        :logger_(logger){}
    virtual ~logable();
    std::shared_ptr<logger> get_logger() {return logger_;}
private:
    std::shared_ptr<logger> logger_;
public:
    logger& log = *logger_;
};

} // namespace logger

#endif // LOGGER_H
