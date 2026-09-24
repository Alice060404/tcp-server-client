cmake_minimum_required(VERSION 3.20)

project(
    tcp-server-client
    VERSION 1.0.0
    LANGUAGES CXX
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Threads REQUIRED)

option(ENABLE_CLANG_TIDY "Run clang-tidy while compiling project targets" ON)
if(ENABLE_CLANG_TIDY)
    find_program(CLANG_TIDY_EXECUTABLE NAMES clang-tidy REQUIRED)
endif()

add_library(network
    src/InetAddress.cpp
    src/Poller.cpp
    src/Socket.cpp
    src/Channel.cpp
    src/EventLoop.cpp
    src/common.cpp
    src/Buffer.cpp
    src/ThreadPool.cpp
    src/Server.cpp
    src/Acceptor.cpp
    src/Connection.cpp
)

target_compile_features(network PUBLIC cxx_std_20)
target_include_directories(network PUBLIC include)
target_link_libraries(network PUBLIC Threads::Threads)

add_executable(echo_server echo_server.cpp)
add_executable(echo_client echo_client.cpp)
add_executable(echo_clients echo_clients.cpp)
add_executable(chat_server chat_server.cpp)
add_executable(chat_client chat_client.cpp)
add_executable(http_server http_server.cpp)

foreach(target IN ITEMS echo_server echo_client echo_clients chat_server chat_client http_server)
    target_link_libraries(${target} PRIVATE network)
endforeach()

add_executable(ThreadPoolTest tests/ThreadPoolTest.cpp)
target_link_libraries(ThreadPoolTest PRIVATE network)

foreach(target IN ITEMS network echo_server echo_client echo_clients chat_server chat_client http_server ThreadPoolTest)
    set_target_properties(${target} PROPERTIES CXX_EXTENSIONS OFF)

    if(ENABLE_CLANG_TIDY)
        set_target_properties(${target} PROPERTIES
            CXX_CLANG_TIDY "${CLANG_TIDY_EXECUTABLE};--quiet"
        )
    endif()

    if(MSVC)
        target_compile_options(${target} PRIVATE /W4)
    else()
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic)
    endif()
endforeach()
