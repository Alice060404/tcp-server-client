# tcp-server-client

用于学习 C++20、TCP 网络编程、事件循环和线程池的示例项目。目前提供 Echo 服务端、交互式客户端、多线程客户端和线程池演示。网络部分使用 Linux `epoll` 与 POSIX socket API，以 Linux / WSL 为运行环境。

服务端监听地址和客户端目标地址定义在 `include/NetworkConfig.hpp`，默认为 `127.0.0.1:8888`。修改 `network_config::IP` 或 `network_config::PORT` 后需要重新构建。

## 项目结构

| 路径 | 作用 |
| --- | --- |
| `include/`、`src/` | `network` 静态库：socket、缓冲区、`Poller`、`EventLoop`、`TCPServer`、连接和线程池 |
| `echo_server.cpp` | Echo 服务端入口 |
| `echo_client.cpp` | 交互式 Echo 客户端 |
| `echo_clients.cpp` | 多线程 Echo 客户端实验程序 |
| `tests/ThreadPoolTest.cpp` | 线程池任务提交演示，不是自动断言测试 |

## 构建

需要 CMake 3.20+、支持 C++20 的编译器和线程库。默认启用 `clang-tidy`；未安装时可在配置命令中加入 `-DENABLE_CLANG_TIDY=OFF`。

```bash
cmake -S . -B build-clang -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_BUILD_TYPE=Debug
cmake --build build-clang --parallel 2
```

构建目标为 `network`、`echo_server`、`echo_client`、`echo_clients` 和 `ThreadPoolTest`。CMake 会缓存编译器；更换编译器时请使用新的构建目录。项目生成 `compile_commands.json`；编译时的 `clang-tidy` 警告不一定导致构建失败。

## 运行

以下命令均从项目根目录执行。先在一个终端启动服务端，再在另一个终端运行客户端。

### Echo

```bash
./build-clang/echo_server
```

```bash
./build-clang/echo_client
```

`echo_client` 逐行读取标准输入，每次发送后读取并打印一次响应。服务端收到数据后将读取缓冲区的内容发回。

### 多线程 Echo 客户端

先启动 `echo_server`，再运行：

```bash
./build-clang/echo_clients -t 1 -m 1
```

| 参数 | 含义 | 默认值 |
| --- | --- | --- |
| `-t` | 客户端任务数及线程池大小 | `100` |
| `-m` | 每个连接的发送与读取轮次 | `100` |
| `-w` | 每个连接建立后、开始收发前等待的秒数 | `0` |

每个任务建立自己的连接，每轮发送 `I am client.`，阻塞读取一次并打印结果。`-w` 对每个连接分别计时。程序使用 `getopt` 和 `std::stoi` 解析参数，目前没有完整的参数校验或收发超时；退出码 `0` 不代表所有回显都正确。当前服务端在客户端断开后可能崩溃，先以小规模参数实验。

### 线程池演示

`./build-clang/ThreadPoolTest` 无需启动服务端，会提交两个打印任务，并在销毁线程池时等待任务完成。多线程打印顺序不固定。CMake 当前没有注册 CTest 测试。

## 当前验证结果与限制

- Clang Debug 构建的全部目标可以编译并链接。实际连接 `127.0.0.1:8888` 时，服务端曾正确回显一次 `hello`。
- 客户端断开后，服务端会发生段错误：EOF 路径删除了仍在事件回调中使用的 `Connection`。因此单次回显成功不代表服务端可以稳定处理完整连接生命周期；并发客户端也不适合用来证明性能或可靠性。
- `Acceptor` 在主 `EventLoop` 上接收连接；`TCPServer` 按 `std::thread::hardware_concurrency()` 创建工作线程和子事件循环，通过 socket 文件描述符取模选择子循环。服务端连接使用非阻塞 socket 和边缘触发事件，客户端使用阻塞收发。
- TCP 是字节流；项目没有定义消息边界。客户端每轮只调用一次 `read()`，不保证读到完整的一条回显。`Buffer` 按 C 字符串处理内容，遇到 `\0` 会截断，不能可靠传输任意二进制数据。
- 非阻塞写入遇到 `EAGAIN` 后尚未保留剩余数据；跨线程共享状态和服务端退出流程仍需完善。
- 子事件循环调用阻塞的事件等待，退出与资源回收流程尚未完整处理；`hardware_concurrency()` 返回 `0` 的情况也未处理。
- `Acceptor` 把必要的 socket 初始化调用写在 `assert` 表达式中；禁用断言的构建不能据此视为已验证可运行。

若客户端提示 `Connection refused`，先确认服务端仍在运行并监听配置的地址和端口。
