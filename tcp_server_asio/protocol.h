#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <memory>
#include "types.h"
#include "../logger/logger.h"

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

struct message
{
    ui16 code;
    ui32 length;
    std::vector<char> data;
};

enum class eTypeProtocol
{
    CommandLine, FileRequest, FileTransmit,
};

class protocol
{
protected:
    std::map<std::string,std::unique_ptr<command>> command_map_;
    eTypeProtocol type_protocol;
public:
    eTypeProtocol get_type_protocol() const {return type_protocol;}
    virtual ~protocol();
    virtual bool recieve(std::shared_ptr<tcp_server::tcp_connection> connection,
                         std::vector<char>&& data) = 0;

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

    bool recieve(std::shared_ptr<tcp_server::tcp_connection> connection,
                 std::vector<char>&& data) override;
    std::optional<std::string> mes_error(eCodeError code_error);
private:
    std::map<eCodeError, std::string> map_message_error_;
    void set_map_message_error();
};

} // namespace tcp_server
