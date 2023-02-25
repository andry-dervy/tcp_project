cmake_minimum_required(VERSION 3.16)

project(tcp_server_asio LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Boost 1.74.0 REQUIRED COMPONENTS
    program_options
    serialization
)

if(Boost_FOUND)
  include_directories(${Boost_INCLUDE_DIRS})
endif()

add_executable(tcp_server_asio
    main.cpp
    tcp_server.h
    tcp_server.cpp
    types.h
    ../logger/logger.h
    ../logger/logger.cpp
    protocol.h
    protocol.cpp
    application.h
    application.cpp
    requestprotocol.h
    requestprotocol.cpp
)

target_link_libraries(tcp_server_asio
    Boost::program_options
    Boost::serialization
)

SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pthread")
