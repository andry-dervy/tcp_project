#include "ui_cli_client.h"

namespace user {

namespace  {

#if defined(_WIN32) || defined(WIN32)
#include <conio.h>
#include <windows.h>
#else
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/ioctl.h>

int _kbhit() {
    static const int STDIN = 0;
    static bool initialized = false;

    if (!initialized) {
        // Use termios to turn off line buffering
        termios term;
        tcgetattr(STDIN, &term);
        term.c_lflag &= ~ICANON;
        tcsetattr(STDIN, TCSANOW, &term);
        setbuf(stdin, NULL);
        initialized = true;
    }

    int bytesWaiting;
    ioctl(STDIN, FIONREAD, &bytesWaiting);
    return bytesWaiting;
}
#endif

}

ui_cli_client::ui_cli_client(tcp_client::tcp_client& client, std::istream& in, std::ostream& out)
    : user_client(client)
    , in_(in), out_(out)
    , rit_prev_command_lines_(prev_command_lines_.rbegin())
    , running(false)
{
    out_ << " Welcome to command line user interface of tcp client!" << std::endl;
}

ui_cli_client::~ui_cli_client()
{
}

void ui_cli_client::start()
{
    running = true;
    while (running)
    {
        input(_kbhit());

        while (!commands_.empty()) {
            auto& com = commands_.front();
            com->execute();
            commands_.pop();
        }

        execute_requests();
    }
    out_ << "Client stop writing." << std::endl;
}

void ui_cli_client::stop()
{
    running = false;
}

void ui_cli_client::execute_requests()
{
    for(auto it = requests_.begin(), ite = requests_.end(); it != ite;)
    {
        switch ((*it)->get_state()) {
        case request_client::eStateRequest::NotReadyToExecute:
            break;
        case request_client::eStateRequest::ReadyToExecute:
            (*it)->execute();
            ++it;
            break;
        case request_client::eStateRequest::WaitRead:
        case request_client::eStateRequest::ReadyToSend:
            ++it;
            break;
        case request_client::eStateRequest::Done:
            it = requests_.erase(it);
            break;
        case request_client::eStateRequest::TimeOut:
            it = requests_.erase(it);
            break;
        }
    }
}

void ui_cli_client::input(int amountInputtedCodes)
{
    assert(amountInputtedCodes >= 0 && amountInputtedCodes < 4);

    if(amountInputtedCodes == 0) return;

#if defined(_WIN32) || defined(WIN32)
    int c = _getch();

    if (c == 224)
    {
        c = _getch();
    }
#else
    uint32_t c = 0;
    uint8_t* p = (uint8_t*)(&c);
    for (int i = 0; i < amountInputtedCodes; ++i)
    {
        *p++ = static_cast<uint8_t>(getchar());
    }
#endif

    //log(std::string(__func__) + " was invoked. key = " + std::to_string(static_cast<int>(c)));

    if(std::isalpha(c))
    {
        command_line_ += c;
        //log(command_line);
        return;
    }

    switch (c)
    {
    case 27: // esc
        commands_.push(make_command<exit_command>(*this));
    return;
    case 10: // enter
    {
        auto&& req = make_request<request_client::universe_request>(out_);
        req->set_data(std::vector<char>(
                          command_line_.begin(),
                          command_line_.end()));
        prev_command_lines_.push_back(std::move(command_line_));
        rit_prev_command_lines_ = prev_command_lines_.rbegin();
        requests_.insert(std::move(req));
    }
    return;
    case 9: // tab
        if(prev_command_lines_.empty()) return;
        command_line_ = *rit_prev_command_lines_;
        commands_.push(make_command<prev_cl_command>(out_, *rit_prev_command_lines_));
        if(rit_prev_command_lines_ != prev_command_lines_.rend() - 1)
        {
            ++rit_prev_command_lines_;
        }
    return;
    }
}

command::~command() {}

void exit_command::execute()
{
    ui_.stop();
}

void prev_cl_command::execute()
{
    out_ << "\n<" << cl_ << std::flush;
}

} // namespace ui
