# tcp-server-client

一个用于学习 Linux TCP 网络编程、事件驱动模型和线程池的 C++20 Echo 客户端/服务端项目。

服务端监听 `127.0.0.1:8888`，采用主从 Reactor 模型：主事件循环接受连接，线程池运行多个子事件循环，处理已建立连接的文本回显。项目包含交互式客户端、多线程并发测试和线程池演示程序。

## 特点

- 使用 C++20 和现代 CMake 组织目标，无第三方库依赖
- 默认启用 clang-tidy，在编译时检查潜在错误和性能问题
- 封装 `Socket`、`InetAddress` 和 `Buffer` 等基础组件
- 通过 `Epoll`、`Channel` 和 `EventLoop` 实现事件分发
- 已建立连接的 socket 使用非阻塞 I/O 与边缘触发模式（`EPOLLET`）
- 使用回调连接 `Acceptor`、`Server` 和 `Connection`
- `ThreadPool` 使用任务队列、互斥锁和条件变量，支持提交任务并返回 `std::future`
- 提供每线程独立连接的并发收发测试

> 本项目依赖 `epoll` 等 Linux API，仅适用于 Linux 或 WSL。

## 项目结构

```text
.
├── include/          # 类与公共接口声明
├── src/              # 网络库及服务端组件实现
├── tests/
│   ├── concurrency_test.cpp  # 多线程并发收发测试
│   └── ThreadPoolTest.cpp    # 线程池任务提交演示
├── client.cpp        # 交互式客户端入口
├── server.cpp        # 服务端入口
├── .clang-tidy       # 静态分析规则
└── CMakeLists.txt    # 构建配置
```

## 运行模型

- 主线程运行主 `EventLoop`；`Acceptor` 使用阻塞监听 socket 和水平触发（LT）接受连接。
- `Server` 根据 `std::thread::hardware_concurrency()` 创建线程池和等量的子 `EventLoop`，每个工作线程运行一个子事件循环。
- 新连接按 socket 文件描述符对事件循环数量取模，分配到对应的子 `EventLoop`。
- 每个事件循环通过 `epoll` 等待事件，由 `Channel` 在当前循环线程中直接调用回调；`Connection` 负责读取与回显。
- 并发测试的 `-t` 仅控制测试客户端线程数，不改变服务端线程池大小。

## 构建

环境要求：

- CMake 3.20 或更高版本
- 支持 C++20 的编译器，以下使用 Clang（本机已验证 18.1.3）
- clang-tidy（默认启用；关闭检查后可不安装）

在项目根目录配置并构建 Debug 版本：

```bash
cmake -S . -B build-clang -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_BUILD_TYPE=Debug
cmake --build build-clang --parallel 2
```

CMake 会缓存编译器选择，因此使用独立的 `build-clang` 目录，不复用原有 GCC 构建目录。后续修改代码后只需执行构建命令。

构建完成后会生成以下可执行文件：

| 文件 | 用途 |
| --- | --- |
| `build-clang/server` | Echo 服务端 |
| `build-clang/client` | 交互式客户端 |
| `build-clang/test` | 多线程并发测试，对应 `tests/concurrency_test.cpp` |
| `build-clang/ThreadPoolTest` | 线程池任务提交演示 |

### clang-tidy 静态检查

`ENABLE_CLANG_TIDY` 默认开启，覆盖库、客户端、服务端和测试目标。规则由根目录 `.clang-tidy` 管理：

- `clang-analyzer-*`：静态分析，例如潜在内存泄漏。
- `bugprone-*`：易出错的代码，例如窄化转换。
- `performance-*`：性能问题，例如不必要的复制。

检查会显示项目源码及 `include/`、`src/` 下头文件的诊断；普通警告不会作为错误，也不会自动修复源码。构建通过不代表没有检查警告。

增量构建仅检查需要重新编译的文件。修改检查规则后，或需要全量检查时执行：

```bash
cmake --build build-clang --clean-first --parallel 2
```

也可利用生成的 `compile_commands.json` 单独检查一个源文件：

```bash
clang-tidy -p build-clang src/Connection.cpp
```

关闭或重新开启构建时检查：

```bash
cmake -S . -B build-clang -DENABLE_CLANG_TIDY=OFF
# 需要恢复时执行
cmake -S . -B build-clang -DENABLE_CLANG_TIDY=ON
```

## 使用

在第一个终端启动服务端：

```bash
./build-clang/server
```

在第二个终端启动客户端：

```bash
./build-clang/client
```

客户端从终端读取一行文本并发送。服务端和客户端均可使用 `Ctrl+C` 结束；测试方式见下文。

## 测试

并发测试需先在另一个终端启动 `./build-clang/server`，无需启动交互式客户端。以下命令均在项目根目录执行。

### 多线程并发测试

`tests/concurrency_test.cpp` 的负载与收发逻辑对齐 `30dayMakeCppServer/code/day14/test/multiple_client.cpp`，目标端口使用本项目的 `8888`，构建目标及可执行文件名仍为 `test`。先创建包含 `-t` 个工作线程的线程池，再提交 `-t` 个客户端任务；每个任务建立一个独立连接，在同一连接上重复收发。参数支持 `-t 100 -m 10` 和 `-t100 -m10` 两种写法。

```bash
# 小规模验证：4 个线程，每线程回显 2 次
./build-clang/test -t 4 -m 2

# 请求 10000 个线程，每线程回显 10 次
./build-clang/test -t 10000 -m 10

# 每条连接建立成功后等待 100 秒，再开始回显测试
./build-clang/test -t 10000 -m 10 -w 100

# 查看参数帮助
./build-clang/test --help
```

在 `build-clang` 目录中可直接使用 `./test -t 10000 -m 10 -w 100`。

| 参数 | 含义 | 要求 / 默认值 |
| --- | --- | --- |
| `-t` | 客户端线程池大小，同时也是客户端任务数 | 正整数，默认 `100` |
| `-m` | 每条连接的回显次数 | 正整数，默认 `100` |
| `-w` | 连接成功后、首次发送前的等待秒数 | 可选，非负整数，默认 `0` |

`-w` 对每条连接独立计时，不是等所有连接建立完成后统一开始。它可用于观察连接保持情况；未指定时，连接成功后立即发送。

- 每轮通过 `Connection` 设置固定消息 `I'm client!`，调用 `write()`，检查连接状态，再调用一次 `read()` 并打印读取缓冲区。
- 每轮打印 `msg count N: ...`，计数在每条连接内从 `0` 开始；不比较回显内容。
- 不设置连接或收发超时；`Connection` 的阻塞收发各调用一次，未保证完整写入或读取整条消息。
- 线程池析构时等待客户端任务结束，随后汇总累计建立的连接数、完成全部收发轮次的任务数、已执行与预期读取轮次、执行比例，以及包含建连和等待的总耗时。
- 保留本项目的参数校验、`--help` 和 `-h`；使用自动对象释放客户端资源。连接数表示累计成功建连次数，不是同时在线的连接峰值；读取轮次不代表成功收到完整回显。

退出码 `0` 不能作为全部回显正确的证明：写入使连接进入 `Closed` 状态时，当前任务会提前结束，但最终仍可能返回 `0`；读取遇到 EOF 时，该轮仍会打印并计数。如果还有下一轮，随后对已关闭连接的 `write()` 会触发断言并终止进程。Socket 创建或连接抛出的异常会打印后以 `1` 退出；参数错误返回 `2`。

大规模测试受客户端线程数、内存、文件描述符、终端输出及本机资源竞争影响；`-t 10000` 不代表服务端已经同时接受 10000 个连接。此程序用于复现 day14 的测试方式，不提供严格的性能或正确性结论。

### 线程池演示

无需启动服务端：

```bash
./build-clang/ThreadPoolTest
```

提交两个打印任务，并在销毁线程池时等待任务完成。它是使用演示，没有自动断言；多线程输出顺序和字符交错不固定。

当前 CMake 未注册 CTest 测试，请直接运行上述可执行文件。在测试命令执行后可查看退出码：

```bash
echo $?
```

若提示 `Connection refused`，请确认服务端已启动且正在监听 `127.0.0.1:8888`。并发测试 `test` 没有收发超时，阻塞时需结合服务端输出排查。

## 当前限制

- IP 地址和端口目前固定为 `127.0.0.1:8888`
- 项目暂未定义独立的应用层消息边界协议；并发测试每轮只读一次，不保证得到完整回显
- 当前按文本处理缓冲区，不支持任意含 `\0` 的二进制数据；发送缓冲区的边界处理仍需完善
- 连接生命周期、共享状态的并发访问，以及非阻塞发送未完成时的处理仍需完善
- 子事件循环尚无退出机制，服务端资源回收流程仍需完善；`hardware_concurrency()` 返回 `0` 的情况尚未处理
- 交互式客户端的部分收发和空输入处理仍需完善，测试程序不覆盖终端交互
- 并发测试用于学习和定位问题，不代表服务端已经通过大规模并发能力验证
