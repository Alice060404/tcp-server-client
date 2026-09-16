#include <algorithm>
#include <array>
#include <cerrno>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <sys/socket.h>
#include <sys/time.h>

#include "InetAddress.hpp"
#include "Socket.hpp"

namespace
{
constexpr std::size_t BUFFER_SIZE = 1024;
constexpr int PORT = 8888;

[[noreturn]] void socketError(const char *operation)
{
    const int error = errno;
    throw std::runtime_error(std::string(operation) + ": " + std::strerror(error));
}

class TestClient
{
  public:
    TestClient()
    {
        const timeval timeout{2, 0};
        for (const int option : {SO_RCVTIMEO, SO_SNDTIMEO})
        {
            if (setsockopt(socket.getFd(), SOL_SOCKET, option, &timeout, sizeof(timeout)) == -1)
                socketError("setsockopt");
        }

        InetAddress address("127.0.0.1", PORT);
        if (connect(socket.getFd(), reinterpret_cast<sockaddr *>(&address.addr), address.addrLen) == -1)
            socketError("connect (请先启动 ./build/server)");
    }

    TestClient(const TestClient &) = delete;
    TestClient &operator=(const TestClient &) = delete;

    void expectEcho(std::string_view message)
    {
        if (message.size() >= BUFFER_SIZE)
            throw std::runtime_error("测试消息过长，需要保留末尾的零字节");

        std::array<char, BUFFER_SIZE> sent{};
        std::array<char, BUFFER_SIZE> received{};
        std::copy(message.begin(), message.end(), sent.begin());

        std::size_t offset = 0;
        while (offset < sent.size())
        {
            const ssize_t count = send(socket.getFd(), sent.data() + offset, sent.size() - offset, MSG_NOSIGNAL);
            if (count < 0)
            {
                if (errno == EINTR)
                    continue;
                socketError("send");
            }
            if (count == 0)
                throw std::runtime_error("send 未发送任何数据");
            offset += static_cast<std::size_t>(count);
        }

        offset = 0;
        while (offset < received.size())
        {
            const ssize_t count = recv(socket.getFd(), received.data() + offset, received.size() - offset, 0);
            if (count < 0)
            {
                if (errno == EINTR)
                    continue;
                socketError("recv (可能是接收超时)");
            }
            if (count == 0)
                throw std::runtime_error("尚未收齐回显，服务端已关闭连接");
            offset += static_cast<std::size_t>(count);
        }

        if (received != sent)
            throw std::runtime_error("回显内容与发送内容不一致");
    }

  private:
    Socket socket;
};
} // namespace

int main()
{
    int passed = 0;
    int failed = 0;
    const auto run = [&](const char *name, auto test) {
        try
        {
            test();
            ++passed;
            std::cout << "[PASS] " << name << std::endl;
        }
        catch (const std::exception &error)
        {
            ++failed;
            std::cerr << "[FAIL] " << name << ": " << error.what() << std::endl;
        }
    };

    run("连接服务端并回显消息", [] {
        TestClient client;
        client.expectEcho("Hello, server!");
    });

    run("同一连接连续收发（英文、中文、空文本）", [] {
        TestClient client;
        client.expectEcho("first message");
        client.expectEcho("你好，TCP！");
        client.expectEcho("");
        client.expectEcho("last message");
    });

    run("多个连接交替收发，消息互不混淆", [] {
        TestClient first;
        first.expectEcho("client A: first");
        TestClient second;
        second.expectEcho("client B: first");
        TestClient third;
        third.expectEcho("client C: first");
        first.expectEcho("client A: second");
        third.expectEcho("client C: second");
        second.expectEcho("client B: second");
    });

    run("客户端断开后，服务端仍可接受新连接", [] {
        {
            TestClient client;
            client.expectEcho("before disconnect");
        }
        TestClient reconnected;
        reconnected.expectEcho("after reconnect");
    });

    std::cout << "结果：" << passed << " 项通过，" << failed << " 项失败。\n";
    return failed == 0 ? 0 : 1;
}
