#pragma once

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/map.hpp>
#include <boost/core/ignore_unused.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/variant.hpp>
#include <boost/mpl/map.hpp>
#include <type_traits>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "protocol.h"
#include "tcp_server.h"

namespace protocol {

enum class eTypeMessage
{
    get_list_files_request,
    get_list_files_answer,
    get_list_dirs_request,
    get_list_dirs_answer,
};

class request
{
public:
    request() {}
    request(std::string type_request) : type_request_(type_request) {}
    virtual ~request() = default;

    virtual std::vector<char> to_sequence();
    virtual void from_sequence(const std::vector<char>& sequential);
    std::string type_request() const;
    void set_type_request(const std::string& type_request);

private:
    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive &ar, const unsigned int version) {
        boost::ignore_unused(version);
        ar & type_request_;
    }

protected:
    std::string type_request_;
};
class answer: public request
{
public:
    answer(std::string type_request) : request(type_request) {}
    app::eError error() const;
    void set_error(const app::eError& error);

    std::string error_string() const;
    void set_error_string(const std::string& error_string);

private:
    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive &ar, const unsigned int version) {
        boost::ignore_unused(version);
        ar & boost::serialization::base_object<request>(*this);
        ar & error_;
        ar & error_string_;
    }

private:
    app::eError error_;
    std::string error_string_;
};
class executor
{
public:
    virtual ~executor() = default;
    virtual std::optional<std::vector<char>> execute(std::vector<char>& sequential) = 0;
};

class get_list_files_request: public request
{
public:
    get_list_files_request() : request("get_list_files") {}
    std::vector<char> to_sequence() override;
    void from_sequence(const std::vector<char>& sequential) override;

    std::string path() const;
    void set_path(const std::string& path);

private:
    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive &ar, const unsigned int version) {
        boost::ignore_unused(version);
        ar & boost::serialization::base_object<request>(*this);
        ar & path_;
    }

private:
    std::string path_;
};
class get_list_files_answer: public answer
{
public:
    get_list_files_answer() : answer("get_list_files") {}
    std::vector<char> to_sequence() override;
    void from_sequence(const std::vector<char>& sequential) override;

    void set_files(const std::vector<std::map<std::string, std::string>>& files);
    std::vector<std::map<std::string, std::string>> get_files() const;

private:
    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive &ar, const unsigned int version) {
        boost::ignore_unused(version);
        ar & boost::serialization::base_object<answer>(*this);
        ar & files_;
    }

private:
    std::vector<std::map<std::string, std::string>> files_; // name_file / type_file
};
class get_list_files_executor: public executor, public logger::logable
{
public:
    get_list_files_executor(app::file_manager_app& fm_app, std::shared_ptr<logger::logger> logger)
        : logger::logable(logger), fm_app_(fm_app) {}
    std::optional<std::vector<char>> execute(std::vector<char>& sequential) override;

private:
    app::file_manager_app& fm_app_;
};

class get_list_dirs_request: public request
{
public:
    get_list_dirs_request(): request("get_list_dirs") {}
    std::vector<char> to_sequence() override;
    void from_sequence(const std::vector<char>& sequential) override;

    std::string path() const;
    void set_path(const std::string& path);

private:
    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive &ar, const unsigned int version) {
        boost::ignore_unused(version);
        ar & boost::serialization::base_object<request>(*this);
        ar & path_;
    }

private:
    std::string path_;
};
class get_list_dirs_answer: public answer
{
public:
    get_list_dirs_answer(): answer("get_list_dirs") {}
    std::vector<char> to_sequence() override;
    void from_sequence(const std::vector<char>& sequential) override;

    void set_dirs(const std::vector<std::map<std::string, std::string>>& dirs);
    std::vector<std::map<std::string, std::string>> get_dirs() const;

private:
    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive &ar, const unsigned int version) {
        boost::ignore_unused(version);
        ar & boost::serialization::base_object<answer>(*this);
        ar & dirs_;
    }

private:
    std::vector<std::map<std::string, std::string>> dirs_; // name_dir / "dir"
};
class get_list_dirs_executor: public executor, public logger::logable
{
public:
    get_list_dirs_executor(app::file_manager_app& fm_app, std::shared_ptr<logger::logger> logger)
        : logger::logable(logger), fm_app_(fm_app) {}
    std::optional<std::vector<char>> execute(std::vector<char>& sequential) override;

private:
    app::file_manager_app& fm_app_;
};

class request_protocol: public protocol, public logger::logable
{
public:
    request_protocol(std::shared_ptr<app::file_manager_app>& app,
                     std::shared_ptr<logger::logger> logger);
    request_protocol(std::shared_ptr<logger::logger> logger);
    bool recieve(std::shared_ptr<tcp_server::tcp_connection> connection,
                 std::vector<char>&& data) override;

    void set_app(std::shared_ptr<app::file_manager_app>& app);

    void fill_request_map();

private:
    std::map<eTypeMessage, std::unique_ptr<executor>> executors_;
    std::shared_ptr<app::file_manager_app> app_;

};

std::shared_ptr<protocol> create_requestprotocol(std::shared_ptr<tcp_server::tcp_server>& srv,
                                                 std::shared_ptr<app::file_manager_app>& app);

} // namespace protocol
