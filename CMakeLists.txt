cmake_minimum_required(VERSION 3.20)

project(
    tcp-server-client
    VERSION 1.0.0
    LANGUAGES CXX
)

# C++ 标准
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 生成 compile_commands.json
# 对 clangd / Neovim 很有用
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# 服务端
add_executable(server
    src/server.cpp
)

# 客户端
add_executable(client
    src/client.cpp
)

target_include_directories(server PRIVATE include)
target_include_directories(client PRIVATE include)

if(MSVC)
    target_compile_options(server PRIVATE /W4)
    target_compile_options(client PRIVATE /W4)
else()
    target_compile_options(server PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(client PRIVATE -Wall -Wextra -Wpedantic)
endif()
