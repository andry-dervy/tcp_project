#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <optional>
#include "boost/asio.hpp"
#include "types.h"
#include "../logger/logger.h"
#include "application.h"

namespace tcp_server {
class tcp_server;
class tcp_connection;
}

namespace protocol
{

enum class eCodeError
{
    Ok,
    CommandLineIsEmpty,
    KeyWordIsNotFound,
    FailedCheck,
};

class command : public logger::logable
{
public:
    command(std::shared_ptr<logger::logger> logger_, std::string key_word)
        : logger::logable(logger_), key_word_(key_word) {}
    virtual ~command();
    virtual eCodeError execute(std::weak_ptr<tcp_server::tcp_connection> connection,
                         std::vector<std::string>& command_line) = 0;
    std::string get_key_word() const {return key_word_;}
private:
    std::string key_word_;
};
class help_command: public command
{
public:
    help_command(std::shared_ptr<logger::logger> logger_)
        : command(logger_, "help") {}
    ~help_command() override;
    eCodeError execute(std::weak_ptr<tcp_server::tcp_connection> connection,
                 std::vector<std::string>& command_line) override;
};
class exit_command: public command
{
public:
    exit_command(std::weak_ptr<tcp_server::tcp_server> srv,std::shared_ptr<logger::logger> logger_)
        : command(logger_, "exit"), tcp_srv_(srv) {}
    ~exit_command() override;
    eCodeError execute(std::weak_ptr<tcp_server::tcp_connection> connection,
                 std::vector<std::string>& command_line) override;

private:
    std::weak_ptr<tcp_server::tcp_server> tcp_srv_;
};

class get_files_command: public command
{
public:
    get_files_command(app::file_manager_app& appl, std::shared_ptr<logger::logger> logger_)
        : command(logger_, "getfiles"), app_(appl) {}
    ~get_files_command() override;
    eCodeError execute(std::weak_ptr<tcp_server::tcp_connection> connection,
                 std::vector<std::string>& command_line) override;

private:
    app::file_manager_app& app_;
};

struct message
{
    ui16 code_;
    ui32 length_;
    std::vector<char> data_;

    std::vector<char> to_sequence() {
        std::vector<char> vout(sizeof(code_) + sizeof(length_) + data_.size());

        ui16 code;
        code.vUI16 = htons(code_.vUI16);
        ui32 length;
        length.vUI32 = htonl(length_.vUI32);

        size_t i {0};
        vout[i++] = code.b1;
        vout[i++] = code.b2;
        vout[i++] = length.b1;
        vout[i++] = length.b2;
        vout[i++] = length.b3;
        vout[i++] = length.b4;

        vout.insert(vout.begin() + i, data_.begin(), data_.end());
        return vout;
    }
};

enum class eTypeProtocol
{
    NoProtocol, CommandLine, Request, FileRequest, FileTransmit,
};

class protocol_base
{
public:
    virtual ~protocol_base() = default;
    virtual bool recieve(std::shared_ptr<tcp_server::tcp_connection> connection,
                         std::vector<char>&& data) = 0;
};

class protocol: public protocol_base
{
protected:
    std::map<std::string,std::unique_ptr<command>> command_map_;
    eTypeProtocol type_protocol;
public:
    eTypeProtocol get_type_protocol() const {return type_protocol;}

    void add_command(std::unique_ptr<command>&& com);
    void del_command(std::string key);

    eCodeError command_execute(std::weak_ptr<tcp_server::tcp_connection> connection,
                               std::vector<std::string>& command_line);
};

class command_line_protocol_state;
class command_line_protocol;

class command_line_protocol_context: public logger::logable
{
public:
    command_line_protocol_context(std::shared_ptr<logger::logger> logger_,
                                  command_line_protocol& ref_clp);
    void set_connection(std::shared_ptr<tcp_server::tcp_connection> connection) {connection_ = connection;}
    void set_data(std::vector<char>&& data) {data_ = data;}
    void set_state(std::unique_ptr<command_line_protocol_state>&& state) {state_ = std::move(state);}
    bool handle();
//private:
    std::shared_ptr<tcp_server::tcp_connection> connection_;
    std::vector<char> data_;
    std::unique_ptr<command_line_protocol_state> state_;
    std::vector<std::string> tokens_;
//private:
    command_line_protocol& ref_clp_;
};

class command_line_protocol_state
{
public:
    virtual ~command_line_protocol_state();
    virtual bool handle(command_line_protocol_context& clp) = 0;
};

class splitting_into_tokens_state: public command_line_protocol_state
{
public:
    ~splitting_into_tokens_state() override;
    bool handle(command_line_protocol_context& clp) override;
};

class command_execute_state: public command_line_protocol_state
{
public:
    ~command_execute_state() override;
    bool handle(command_line_protocol_context& clp) override;
};

class command_line_protocol: public protocol, public logger::logable
{
public:
    command_line_protocol(std::shared_ptr<logger::logger> logger);
    command_line_protocol();
    ~command_line_protocol() override;
    void set_application(std::shared_ptr<app::file_manager_app>& app) {app_ = app;}

    bool recieve(std::shared_ptr<tcp_server::tcp_connection> connection,
                 std::vector<char>&& data) override;
    std::optional<std::string> mes_error(eCodeError code_error);
private:
    std::map<eCodeError, std::string> map_message_error_;
    void set_map_message_error();
    std::shared_ptr<app::file_manager_app> app_;
};

std::shared_ptr<protocol> create_command_line_protocol(std::shared_ptr<tcp_server::tcp_server>& srv,
                                                       std::shared_ptr<app::file_manager_app>& app);

} // namespace tcp_server
