#include <iostream>
#include <fstream>
#include "boost/asio.hpp"
#include "tcp_client.h"
#include "boost/program_options.hpp"
#include "../logger/logger.h"
#include "ui_cli_client.h"

namespace opt = boost::program_options;

void run_client(std::string address, int port)
{
    auto os = std::ofstream("log_error.txt", std::ios_base::out);
    std::shared_ptr<logger::logger> log;

    if(!os)
    {
        log = std::make_shared<logger::logger>(std::cout);
    }
    else
    {
        log = std::make_shared<logger::logger>(os);
    }

    try
    {
        auto ui_client = std::make_unique<user::ui_cli_client>(tcp_client::tcp_client::instance(), std::cin, std::cout);

        ui_client->set_address(address);
        ui_client->set_port(port);

        ui_client->start();
    }
    catch (std::exception& e)
    {
        log->log(e.what());
        os.close();
    }
}

int main(int argc, char* argv[])
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
        run_client(address, port);
    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}
