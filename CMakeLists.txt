cmake_minimum_required(VERSION 3.5)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(logger)
add_subdirectory(tcp_server_asio)
add_subdirectory(tcp_client_asio)
add_subdirectory(gui_client)
