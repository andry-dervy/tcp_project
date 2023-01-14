cmake_minimum_required(VERSION 3.5)

project(tcp_client_asio LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Boost 1.71.0 REQUIRED COMPONENTS
    program_options)

if(Boost_FOUND)
  include_directories(${Boost_INCLUDE_DIRS})
endif()

add_executable(tcp_client_asio
    main.cpp
    tcp_client.h
    tcp_client.cpp
    ui_cli_client.h
    ui_cli_client.cpp
    ../logger/logger.h
    ../logger/logger.cpp
)

target_link_libraries(tcp_client_asio Boost::program_options)

SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pthread")
