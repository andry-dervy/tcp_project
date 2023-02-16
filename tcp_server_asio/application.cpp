
#include <iostream>
#include <filesystem>
#include "application.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

namespace app {

file_manager_app::file_manager_app(std::shared_ptr<logger::logger> logger)
    : logger::logable(logger)
    , rootdir_("")
    , error_(eError::no_error)
    , error_string_("")
{}

void file_manager_app::load_settings()
{
    boost::property_tree::ptree pt;
    boost::property_tree::json_parser::read_json("init.json", pt);
    boost::optional<std::string> opt_rootdir = pt.get_optional<std::string>("rootdir");
    if(opt_rootdir.has_value()) rootdir_ = *opt_rootdir;

    std::cout << "Root directory: " << rootdir_ << std::endl;
}

std::vector<std::string> file_manager_app::get_files(std::string path)
{
    auto path_dir = rootdir_ + path;
    if(!check_path(path_dir)) return std::vector<std::string>();

    std::vector<std::string> v_namefiles;

    for (auto const& dir_entry : std::filesystem::directory_iterator{path_dir})
    {
        if(dir_entry.is_regular_file())
        {
            log(dir_entry.path().filename());
            v_namefiles.push_back(dir_entry.path().filename());
        }
    }

    return v_namefiles;
}

std::vector<std::string> file_manager_app::get_directories(std::string path)
{
    auto path_dir = rootdir_ + path;
    if(!check_path(path_dir)) return std::vector<std::string>();

    std::vector<std::string> v_namedirs;

    for (auto const& dir_entry : std::filesystem::directory_iterator{path_dir})
    {
        if(dir_entry.is_directory()) v_namedirs.push_back(dir_entry.path().filename());
    }

    return v_namedirs;
}

eError file_manager_app::get_error() const
{
    return error_;
}

std::string file_manager_app::get_error_string() const
{
    return error_string_;
}

bool file_manager_app::check_path(std::string& path)
{
    if(std::filesystem::exists(path))
    {
        std::cout << path << " exists." << std::endl;
        error_ = eError::no_error;
        error_string_ = "";
        return true;
    }
    else
    {
        error_string_ = path + " do not exists.";
        std::cout << error_string_ << std::endl;
        error_ = eError::do_not_exists;
        return false;
    }
}

} // namespace app

