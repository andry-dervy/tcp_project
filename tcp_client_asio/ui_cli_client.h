#pragma once

#include <iostream>
#include <memory>
#include <queue>
#include <thread>
#include "../logger/logger.h"
#include "user_client.h"
#include "tcp_client.h"
#include "request.h"

namespace user {

class ui_cli_client;

class command
{
public:
    virtual ~command();
    virtual void execute() = 0;
};

class exit_command: public command
{
public:
    exit_command(ui_cli_client& ui) : ui_(ui) {}
    void execute() override;
private:
    ui_cli_client& ui_;
};

class prev_cl_command: public command
{
public:
    prev_cl_command(std::ostream& out, std::string cl) : out_(out), cl_(cl) {}
    void execute() override;
private:
    std::ostream& out_;
    std::string cl_;
};

class ui_cli_client: public user_client
{
public:
    ui_cli_client(tcp_client::tcp_client& client, std::istream& in, std::ostream& out);
    ~ui_cli_client() override;
    void start();
    void stop();

private:
    std::istream& in_;
    std::ostream& out_;

    std::queue<std::unique_ptr<command>> commands_;

    std::string command_line_;
    std::vector<std::string> prev_command_lines_;
    std::vector<std::string>::reverse_iterator rit_prev_command_lines_;

    template<typename T, typename... Args>
    std::unique_ptr<command> make_command(Args&... args) {
        log("make_command");
        return std::make_unique<T>(args...);
    }

private:
    void input(int amountInputtedCodes);
    void execute_requests();
protected:
    bool running;
};

} // namespace ui
