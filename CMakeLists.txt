cmake_minimum_required(VERSION 3.20)

project(
    tcp-server-client
    VERSION 1.0.0
    LANGUAGES CXX
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(network
    src/InetAddress.cpp
    src/Epoll.cpp
    src/Socket.cpp
    src/Channel.cpp
    src/EventLoop.cpp
    src/common.cpp
)

target_compile_features(network PUBLIC cxx_std_20)
target_include_directories(network PUBLIC include)

add_executable(server
    server.cpp
    src/Server.cpp
)
target_link_libraries(server PRIVATE network)

add_executable(client client.cpp)
target_link_libraries(client PRIVATE network)

foreach(target IN ITEMS network server client)
    set_target_properties(${target} PROPERTIES CXX_EXTENSIONS OFF)

    if(MSVC)
        target_compile_options(${target} PRIVATE /W4)
    else()
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic)
    endif()
endforeach()
