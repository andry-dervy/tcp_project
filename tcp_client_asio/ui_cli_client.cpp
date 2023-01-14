#include "ui_cli_client.h"

namespace ui {

ui_client::~ui_client() {}

ui_cli_client::ui_cli_client(std::shared_ptr<logger::logger> logger, std::istream& in, std::ostream& out)
    :logger::logable(logger), in_(in), out_(out)
{
    out_ << " Welcome to command line user interface of tcp client!" << std::endl;
}

void ui_cli_client::start()
{
    client_->connect();

    while (true)
    {
        std::string request;
        out_ << "\n> " << std::flush;
        std::fflush(stdout);
        if (!std::getline(in_, request)) break;

        out_ << "Sending request: \"" << request << "\"..." << std::endl;

        client_->write(request);

        boost::system::error_code error;

        auto mes = client_->read(error);
        if (error == boost::asio::error::eof)
            break; // Connection closed cleanly by peer.
        else if (error)
            throw boost::system::system_error(error); // Some other error.

        out_ << "Received message: " << mes;
    }
    out_ << "Client stop writing." << std::endl;
}

} // namespace ui
