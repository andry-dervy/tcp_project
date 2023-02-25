cmake_minimum_required(VERSION 3.5)

project(gui_client LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# QtCreator supports the following variables for Android, which are identical to qmake Android variables.
# Check https://doc.qt.io/qt/deployment-android.html for more information.
# They need to be set before the find_package( ...) calls below.

#if(ANDROID)
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
#    if (ANDROID_ABI STREQUAL "armeabi-v7a")
#        set(ANDROID_EXTRA_LIBS
#            ${CMAKE_CURRENT_SOURCE_DIR}/path/to/libcrypto.so
#            ${CMAKE_CURRENT_SOURCE_DIR}/path/to/libssl.so)
#    endif()
#endif()

find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets REQUIRED)

find_package(Boost 1.74.0 REQUIRED COMPONENTS
    program_options
    system
    thread
    serialization
)

if(Boost_FOUND)
  include_directories(${Boost_INCLUDE_DIRS})
endif()

set(PROJECT_SOURCES
        main.cpp
        src/mainwindow.cpp
        src/mainwindow.h
        src/server_settings_dialog.cpp
        src/server_settings_dialog.h
        src/api_client.cpp
        src/api_client.h
        src/tab.cpp
        src/tab.h
        ../logger/logger.h
        ../logger/logger.cpp
        ../tcp_client_asio/request.h
        ../tcp_client_asio/request.cpp
        ../tcp_client_asio/tcp_client.h
        ../tcp_client_asio/tcp_client.cpp
        ../tcp_client_asio/user_client.h
        ../tcp_client_asio/user_client.cpp
        ../tcp_server_asio/requestprotocol.h
        ../tcp_server_asio/requestprotocol.cpp
        ../tcp_server_asio/protocol.h
        ../tcp_server_asio/protocol.cpp
        ../tcp_server_asio/application.h
        ../tcp_server_asio/application.cpp
        ../tcp_server_asio/tcp_server.h
        ../tcp_server_asio/tcp_server.cpp
        ../tcp_server_asio/types.h
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(gui_client
        ${PROJECT_SOURCES}
    )
else()
    if(ANDROID)
        add_library(gui_client SHARED
            ${PROJECT_SOURCES}
        )
    else()
        add_executable(gui_client
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(gui_client PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Boost::program_options
    Boost::system
    Boost::thread
    Boost::serialization
)

SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pthread")
