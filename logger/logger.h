#ifndef LOGGER_H
#define LOGGER_H

#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <sstream>
#include <memory>
#include <mutex>
#include <fstream>

namespace logger {

class stream
{
public:
    stream(std::ostream &ostream) : ostream_(ostream) {}
    virtual ~stream() = default;
protected:
    std::ostream &ostream_;
};

class file
{
public:
    file(const std::string& file_name);
    virtual ~file();
    operator bool() const {return is_open;}
protected:
    std::ofstream os_;
    bool is_open;
};

class logger: public stream
{
public:
    logger(std::ostream &ostream);
    virtual ~logger() = default;

    void operator()(std::string &msg);
    void operator()(std::string &&msg);
    void log(std::string &msg);
    void log(std::string &&msg);

    std::ostream& operator()() {return ostream_;}

private:
    std::mutex mtx_;
private:
    void log_(std::string &msg);
};

class file_logger: public file, public logger
{
public:
    file_logger(const std::string& file_name);
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
