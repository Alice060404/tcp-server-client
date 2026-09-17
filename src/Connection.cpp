#include "Connection.hpp"
#include "Channel.hpp"
#include "EventLoop.hpp"
#include "Socket.hpp"
#include <array>
#include <cstddef>
#include <functional>
#include <iostream>
#include <sys/types.h>
#include <unistd.h>

constexpr std::size_t BUFFER_SIZE = 1024;

Connection::Connection(EventLoop *_loop, Socket *_sock) : loop(_loop), sock(_sock)
{
    channel = new Channel(loop, sock->getFd());
    std::function<void()> cb = std::bind(&Connection::echo, this, sock->getFd());
    channel->setCallback(cb);
    channel->enableReading();
}

Connection::~Connection()
{
    delete sock;
    delete channel;
}

void Connection::echo(int sockFd)
{
    std::array<char, BUFFER_SIZE> buffer{};
    while (true)
    {
        buffer = {};

        const ssize_t readBytes = read(sockFd, buffer.data(), buffer.size());
        if (readBytes > 0)
        {
            std::cout << "Msg from client fd " << sockFd << ": " << buffer.data() << '\n';
            write(sockFd, buffer.data(), buffer.size());
        }
        else if (readBytes == 0)
        {
            std::cout << "Client fd " << sockFd << " disconnect.\n";
            deleteConnectionCallback(sock);
            break;
        }
        else if (readBytes == -1 && errno == EINTR)
        {
            std::cout << "Continue read.\n";
            continue;
        }
        else if (readBytes == -1 && ((errno == EAGAIN) || (errno == EWOULDBLOCK)))
        {
            std::cout << "Finish reading once, errno: " << errno << '\n';
            break;
        }
    }
}

void Connection::setDeleteConnectionCallback(std::function<void(Socket *)> _cb)
{
    deleteConnectionCallback = _cb;
}
