# tcp-server-client

用于学习 C++20、TCP 网络编程、事件循环和线程池的示例项目。当前提供 Echo、聊天广播的实验入口。服务端统一监听 `127.0.0.1:8888`；同一时间只能启动其中一个服务端。

项目使用 Linux `epoll` 和 POSIX socket API，当前构建与运行以 Linux / WSL 为准。`Poller` 中虽然有 macOS `kqueue` 分支，但其实现尚未完成验证。

服务端监听地址和示例客户端的目标地址统一定义在 `include/NetworkConfig.hpp`。修改其中的 `network_config::IP` 和 `network_config::PORT` 后，重新构建即可让这些入口使用新的地址。

## 项目结构

| 路径 | 作用 |
| --- | --- |
| `include/`、`src/` | `network` 静态库：socket、缓冲区、事件分发、连接、服务端和线程池 |
| `echo_server.cpp`、`echo_client.cpp` | Echo 服务端与交互式客户端 |
| `echo_clients.cpp` | 多线程 Echo 客户端负载程序 |
| `chat_server.cpp`、`chat_client.cpp` | 广播服务端与仅接收消息的客户端 |
| `http_server.cpp` | 使用 `Server` 的回显示例；当前未解析或生成 HTTP 报文 |
| `tests/ThreadPoolTest.cpp` | 线程池任务提交演示，不是自动断言测试 |

## 构建

需要 CMake 3.20+、支持 C++20 的 C++ 编译器和线程库。CMake 默认开启 `ENABLE_CLANG_TIDY`，因此还需要安装 `clang-tidy`；没有该工具时，可在配置时指定 `-DENABLE_CLANG_TIDY=OFF`。

```bash
cmake -S . -B build-clang -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_BUILD_TYPE=Debug
cmake --build build-clang --parallel 2
```

生成的可执行文件位于 `build-clang/`：`echo_server`、`echo_client`、`echo_clients`、`chat_server`、`chat_client`、`http_server` 和 `ThreadPoolTest`。CMake 会缓存编译器；更换编译器时请使用新的构建目录。项目会生成 `compile_commands.json`，`.clang-tidy` 配置了 `clang-analyzer-*`、`bugprone-*` 和 `performance-*` 检查。构建时的检查告警不一定导致构建失败。

## 运行

以下命令均从项目根目录执行。先在一个终端启动服务端，再在其他终端运行对应客户端。

### Echo

```bash
./build-clang/echo_server
```

```bash
./build-clang/echo_client
```

`echo_client` 从标准输入逐行读取文本，发送后读取并打印一次服务端响应。`echo_server` 收到数据后将读取缓冲区的内容发回。服务端注册了 `SIGINT` 处理函数，可用 `Ctrl+C` 请求退出；当前关闭流程仍有局限，见下文。

### 多线程 Echo 客户端

先启动 `echo_server`，再运行：

```bash
./build-clang/echo_clients -t 4 -m 2
```

| 参数 | 含义 | 默认值 |
| --- | --- | --- |
| `-t` | 客户端线程池大小和提交的客户端任务数 | `100` |
| `-m` | 每个连接的发送与读取轮次 | `100` |
| `-w` | 每个连接建立后、开始收发前等待的秒数 | `0` |

每个任务建立自己的连接，每轮发送 `I am client.`，阻塞读取一次并打印结果。`-w` 对每个连接分别计时。程序使用 `getopt` 和 `std::stoi` 解析参数，目前没有完整的参数校验或 `--help` 帮助；建议先使用小规模参数。它不会比较收到的内容与发出的内容，也没有收发超时，因此退出码 `0` 不代表所有回显都正确。

### 聊天示例

```bash
./build-clang/chat_server
```

```bash
./build-clang/chat_client
```

`chat_server` 将收到的内容发送给其保存的客户端连接。`chat_client` 只循环接收并打印消息，没有从终端发送消息的逻辑；可以使用 `echo_client` 作为发送端进行简单观察。当前服务端没有从广播列表移除断开的连接，也没有对共享列表加锁，聊天示例不适合并发可靠性测试。

### 其他入口与线程池演示

`./build-clang/http_server` 当前调用与 Echo 示例相同的 `Server` 和回显逻辑，只是没有新连接日志。

`./build-clang/ThreadPoolTest` 无需启动服务端，会提交两个打印任务，并在销毁线程池时等待任务完成。多线程打印顺序不固定。CMake 当前没有注册 CTest 测试。

## 运行模型与已知限制

- `Acceptor` 在主 `EventLoop` 上接收连接；`Server` 按 `std::thread::hardware_concurrency()` 创建工作线程和子事件循环，通过 socket 文件描述符取模选择子循环。服务端连接使用非阻塞 socket 和边缘触发事件，客户端使用阻塞收发。
- TCP 是字节流；项目没有定义消息边界。客户端每轮只调用一次 `read()`，不保证读到完整的一条回显。`Buffer` 按 C 字符串处理内容，遇到 `\0` 会截断，不能可靠传输任意二进制数据。
- 非阻塞写入遇到 `EAGAIN` 后尚未保留剩余数据；连接关闭、回调生命周期以及跨线程共享状态的处理仍需完善。不要把本项目的多线程程序当作吞吐量或并发能力的合格证明。
- 子事件循环调用阻塞的事件等待，退出与资源回收流程尚未完整处理；`hardware_concurrency()` 返回 `0` 的情况也未处理。

若客户端提示 `Connection refused`，先确认对应服务端已启动并监听 `127.0.0.1:8888`。
