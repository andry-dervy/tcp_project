#include <iostream>
#include "boost/asio.hpp"
#include "boost/program_options.hpp"

#include "application.h"
#include "tcp_server.h"
#include "../logger/logger.h"
#include "requestprotocol.h"

namespace opt = boost::program_options;

void run_server(std::string address, int port, std::string mode)
{
    auto app_logger {std::make_shared<logger::file_logger>("app_log.txt")};
    auto application {std::make_shared<app::file_manager_app>(app_logger)};

    application->load_settings();

    auto logger {std::make_shared<logger::logger>(std::cout)};

    tcp_server::io_context_t io_context;
    auto server = std::make_shared<tcp_server::tcp_server>(logger, io_context, address, port);
    if(mode == "request")
        server->add_protocol(protocol::create_requestprotocol(server, application));
    else if(mode == "command_line")
        server->add_protocol(protocol::create_command_line_protocol(server, application));

    io_context.run();
}

int main(int argc, char *argv[])
{
    std::string address;
    int port;
    std::string mode;

    opt::options_description desc("All options");
    desc.add_options()
            ("address,a", opt::value<std::string>(&address)->default_value("127.0.0.1"), "server address")
            ("port,p", opt::value<int>(&port)->default_value(13000), "server port")
            ("help", "produce help message")
            ("mode,m", opt::value<std::string>(&mode)->default_value("request"), "interpretation mode of received messages")
            ;

    // Переменная для хранения аргументов нашей командной строки
    opt::variables_map vm;
    // Парсинг и сохранение аргументов
    opt::store(opt::parse_command_line(argc, argv, desc), vm);
    // Эта функция должна вызываться после парсинга и сохранения.

    if (vm.count("help")) {
        std::cout << desc << "\n";
        return 1;
    }

    try {
    // Если один из обязательных параметров не был задан, выбрасывается исключение
    // `opt::required_option`
        opt::notify(vm);
    } catch (const opt::required_option& e) {
        std::cout << "Error: " << e.what() << std::endl;
        return 2;
    }

    try
    {
        run_server(address, port, mode);
    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}
