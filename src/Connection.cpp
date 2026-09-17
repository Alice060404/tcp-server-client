#include <array>
#include <cstddef>
#include <functional>
#include <iostream>
#include <sys/types.h>
#include <unistd.h>

#include "Buffer.hpp"
#include "Channel.hpp"
#include "Connection.hpp"
#include "EventLoop.hpp"
#include "Socket.hpp"
#include "common.hpp"

constexpr std::size_t BUFFER_SIZE = 1024;

Connection::Connection(EventLoop *_loop, Socket *_sock)
    : loop(_loop), sock(_sock), inBuffer(new std::string()), readBuffer(nullptr)
{
    channel = new Channel(loop, sock->getFd());
    std::function<void()> cb = std::bind(&Connection::echo, this, sock->getFd());
    channel->setCallback(cb);
    channel->enableReading();
    readBuffer = new Buffer();
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
            readBuffer->append(buffer.data(), readBytes);
        }
        else if (readBytes == 0)
        {
            std::cout << "Client fd " << sockFd << " disconnect.\n";
            deleteConnectionCallback(sock);
            break;
        }
        else if (readBytes == -1 && errno == EINTR)
        {
            std::cout << "Continue reading.\n";
            continue;
        }
        else if (readBytes == -1 && ((errno == EAGAIN) || (errno == EWOULDBLOCK)))
        {
            std::cout << "Finish reading once, errno: " << errno << '\n';
            std::cout << "Msg from client fd " << sockFd << " : " << readBuffer->c_str() << '\n';
            common::exception::throw_if(write(sockFd, readBuffer->c_str(), readBuffer->size()) == -1,
                                        "Sock write error.");
            readBuffer->clear();
            break;
        }
    }
}

void Connection::setDeleteConnectionCallback(std::function<void(Socket *)> _cb)
{
    deleteConnectionCallback = _cb;
}
