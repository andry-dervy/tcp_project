#include <iostream>
#include "boost/asio.hpp"
#include "tcp_server.h"
#include "../logger/logger.h"
#include "boost/program_options.hpp"

namespace opt = boost::program_options;

std::unique_ptr<protocol::protocol> create_command_line_protocol(std::shared_ptr<tcp_server::tcp_server> srv)
{
    auto protocol = std::make_unique<protocol::command_line_protocol>(srv->get_logger());

    protocol->add_command(std::make_unique<protocol::exit_command>(srv, srv->get_logger()));
    protocol->add_command(std::make_unique<protocol::help_command>(srv->get_logger()));

    return protocol;
}

void run_server(std::string address, int port)
{
    auto logger {std::make_shared<logger::logger>(std::cout)};

    tcp_server::io_context_t io_context;
    auto server = std::make_shared<tcp_server::tcp_server>(logger, io_context, address, port);
    server->add_protocol(create_command_line_protocol(server));

    io_context.run();
}

int main(int argc, char *argv[])
{
    std::string address;
    int port;

    opt::options_description desc("All options");
    desc.add_options()
            ("address,a", opt::value<std::string>(&address)->default_value("127.0.0.1"), "server address")
            ("port,p", opt::value<int>(&port)->default_value(13000), "server port")
            ("help", "produce help message");

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
        run_server(address, port);
    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}
