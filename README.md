# tcp-server-client

一个用于学习 Linux TCP 网络编程与事件驱动模型的 C++20 Echo 客户端/服务端项目。

服务端通过单线程事件循环监听 `127.0.0.1:8888`，将收到的数据原样返回；交互式客户端从终端读取消息，并显示服务端的响应。

## 特点

- 使用 C++20 和现代 CMake 组织目标
- 封装 `Socket`、`InetAddress` 和 `Buffer` 等基础组件
- 通过 `Epoll`、`Channel` 和 `EventLoop` 实现事件分发
- 使用非阻塞 socket 与边缘触发模式（`EPOLLET`）
- 使用回调连接 `Acceptor`、`Server` 和 `Connection`
- 支持多个客户端连接及连接断开后的资源清理
- 提供不依赖第三方测试框架的 TCP 功能测试

> 本项目依赖 `epoll` 等 Linux API，仅适用于 Linux 或 WSL。

## 项目结构

```text
.
├── include/          # 类与公共接口声明
├── src/              # 网络库及服务端组件实现
├── tests/            # TCP 功能测试
├── client.cpp        # 交互式客户端入口
├── server.cpp        # 服务端入口
└── CMakeLists.txt    # 构建配置
```

## 构建

环境要求：

- CMake 3.20 或更高版本
- 支持 C++20 的编译器

在项目根目录配置并构建：

```bash
cmake -S . -B build
cmake --build build
```

构建完成后会生成：

- `build/server`
- `build/client`
- `build/tcp_test`

## 使用

在第一个终端启动服务端：

```bash
./build/server
```

在第二个终端启动客户端：

```bash
./build/client
```

连接成功后，输入一行文本并按 Enter，客户端会显示服务端返回的内容：

```text
Msg from server: Hello
```

可以同时启动多个客户端。服务端和客户端均可使用 `Ctrl+C` 结束。

## 测试

功能测试覆盖以下场景：

- 建立连接并回显消息
- 在同一连接中连续收发
- 三个客户端交替收发
- 客户端断开后重新连接

先启动服务端并保持运行：

```bash
./build/server
```

再在另一个终端运行测试程序，无需启动交互式客户端：

```bash
./build/tcp_test
```

测试程序以退出码 `0` 表示全部通过，可以使用以下命令查看：

```bash
echo $?
```

若提示 `Connection refused`，请确认服务端已启动且正在监听 `127.0.0.1:8888`。

## 当前范围

- IP 地址和端口目前固定为 `127.0.0.1:8888`
- 功能测试的收发超时为 2 秒
- 功能测试不覆盖压力测试、异常网络环境或交互式终端输入
- 项目暂未定义独立的应用层消息边界协议
