cmake_minimum_required(VERSION 3.5)

project(tcp_client_asio LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Boost 1.74.0 REQUIRED COMPONENTS
    program_options
    system
    thread
)

if(Boost_FOUND)
  include_directories(${Boost_INCLUDE_DIRS})
endif()

add_executable(tcp_client_asio
    main.cpp
    tcp_client.h
    tcp_client.cpp
    ui_cli_client.h
    ui_cli_client.cpp
    user_client.h
    user_client.cpp
    request.h
    request.cpp
    ../logger/logger.h
    ../logger/logger.cpp
)

target_link_libraries(tcp_client_asio
    Boost::program_options
    Boost::system
    Boost::thread
)

SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pthread")
