#pragma once

#include <string>
#include <vector>
#include "../logger/logger.h"

namespace app {

enum class eError
{
    no_error,
    do_not_exists,
};

class file_manager_app: public logger::logable
{
public:
    file_manager_app(std::shared_ptr<logger::logger> logger);
    void load_settings();
    std::vector<std::string> get_files(std::string path);
    std::vector<std::string> get_directories(std::string path);

    eError get_error() const;
    std::string get_error_string() const;

private:
    std::string rootdir_;
    eError error_;
    std::string error_string_;

private:
    bool check_path(std::string& path);
};

} // namespace app
