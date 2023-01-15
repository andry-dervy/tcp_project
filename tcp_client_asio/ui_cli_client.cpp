#include "ui_cli_client.h"

namespace ui {

ui_client::~ui_client() {}

ui_cli_client::ui_cli_client(std::shared_ptr<logger::logger> logger, std::istream& in, std::ostream& out)
    :logger::logable(logger), in_(in), out_(out)
{
    out_ << " Welcome to command line user interface of tcp client!" << std::endl;
}

ui_cli_client::~ui_cli_client()
{
    if(thread)
    {
        tcp_client::tcp_client::stop();
    }
    out_ << " ui_cli_client destruction." << std::endl;
}

void tcp_client_start()
{
    tcp_client::tcp_client::start();
}

void ui_cli_client::start()
{
    std::thread thread(tcp_client_start);
    thread.detach();

    while (true)
    {
        std::string request;
        out_ << "\n> " << std::flush;
        std::fflush(stdout);
        if (!std::getline(in_, request)) break;

        auto handle_request = std::make_unique<universe_request_callback>(out_);
        client_->request(std::vector<char>(request.begin(),request.end()), *handle_request);
    }
    out_ << "Client stop writing." << std::endl;
}

request_callback::~request_callback() {}

universe_request_callback::~universe_request_callback() {}

tcp_client::connection_ptr&& universe_request_callback::on_send(tcp_client::connection_ptr&& c) const
{
    out_ << "Send request: \"" << std::string(c->data.begin(), c->data.end()) << "\"..." << std::endl;
    return std::move(c);
}

tcp_client::connection_ptr&& universe_request_callback::on_read(tcp_client::connection_ptr&& c) const
{
    out_ << "Read respose: \"" << std::string(c->data.begin(), c->data.end()) << "\"..." << std::endl;
    return std::move(c);
}

} // namespace ui
