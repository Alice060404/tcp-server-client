# tcp-server-client

一个用于学习 Linux TCP 网络编程的 C++20 Echo 客户端/服务端示例。

服务端监听 `127.0.0.1:8888`，接收客户端发送的消息并原样返回；客户端从标准输入读取消息，并输出服务端的响应。

## 特点

- 使用 C++20 和 CMake 构建
- 对 `Socket`、`InetAddress`、`Epoll` 和 `Channel` 进行基础封装
- 服务端使用非阻塞 socket
- 使用 Linux `epoll` 进行 I/O 多路复用
- 使用边缘触发模式（`EPOLLET`）处理读事件
- 支持多个客户端连接

> 本项目依赖 Linux 网络 API（如 `epoll`），适用于 Linux 或 WSL 环境。

## 构建

环境要求：

- CMake 3.20 或更高版本
- 支持 C++20 的编译器

在项目根目录执行：

```bash
cmake -S . -B build
cmake --build build
```

构建完成后，可执行文件位于 `build/` ：

- `build/server`
- `build/client`
- `build/tcp_test`

## 使用

先在一个终端启动服务端：

```bash
./build/server
```

再在另一个终端启动客户端：

```bash
./build/client
```

客户端连接成功后，输入一行文本并按 Enter。服务端会输出收到的消息，客户端会显示服务端返回的内容：

```text
Msg from server: Hello
```

可启动多个客户端进行连接测试。按 `Ctrl+C` 结束程序。

## 测试

`tcp_test` 使用 C++ 标准库和 Linux socket API。

启动服务端，监听 `127.0.0.1:8888`，测试期间保持运行。

```bash
./build/server
```

运行测试程序，无需另外启动 `client` 或手动输入消息。

```bash
./build/tcp_test
```

若提示 `Connection refused`，先确认服务端已成功启动。测试完成后，在终端一按 `Ctrl+C` 停止服务端。

收发超时为 2 秒。测试使用当前客户端的 1024 字节补零缓冲区格式，不覆盖任意长度消息、并发连接压力或交互式客户端的终端输入。
