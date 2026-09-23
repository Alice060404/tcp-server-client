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
    src/Epoll.cpp
    src/Socket.cpp
    src/Channel.cpp
    src/EventLoop.cpp
    src/common.cpp
    src/Buffer.cpp
    src/ThreadPool.cpp
)

target_compile_features(network PUBLIC cxx_std_20)
target_include_directories(network PUBLIC include)
target_link_libraries(network PUBLIC Threads::Threads)

add_executable(server
    server.cpp
    src/Server.cpp
    src/Acceptor.cpp
    src/Connection.cpp
)
target_link_libraries(server PRIVATE network)

add_executable(client client.cpp src/Connection.cpp)
target_link_libraries(client PRIVATE network)

add_executable(test tests/concurrency_test.cpp src/Connection.cpp)
target_link_libraries(test PRIVATE network)

add_executable(ThreadPoolTest tests/ThreadPoolTest.cpp)
target_link_libraries(ThreadPoolTest PRIVATE network)

foreach(target IN ITEMS network server client test ThreadPoolTest)
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
