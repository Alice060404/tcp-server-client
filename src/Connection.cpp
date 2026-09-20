#include <array>
#include <cerrno>
#include <cstddef>
#include <functional>
#include <iostream>
#include <memory.h>
#include <sys/types.h>
#include <unistd.h>

#include "Buffer.hpp"
#include "Channel.hpp"
#include "Connection.hpp"
#include "EventLoop.hpp"
#include "Socket.hpp"

constexpr std::size_t BUFFER_SIZE = 1024;

Connection::Connection(EventLoop *_loop, Socket *_sock)
    : loop(_loop), sock(_sock), channel(nullptr), inBuffer(new std::string()), readBuffer(nullptr)
{
    channel = new Channel(loop, sock->getFd());
    std::function<void()> cb = std::bind(&Connection::echo, this, sock->getFd());
    channel->enableReading();
    channel->useET();
    channel->setReadCallback(cb);
    channel->setUseThreadPool(true);
    readBuffer = new Buffer();
}

Connection::~Connection()
{
    delete channel;
    delete sock;
    delete readBuffer;
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
            deleteConnectionCallback(sockFd);
            break;
        }
        else if (readBytes == -1 && errno == EINTR)
        {
            std::cout << "Continue reading.\n";
            continue;
        }
        else if (readBytes == -1 && ((errno == EAGAIN) || (errno == EWOULDBLOCK)))
        {
            std::cout << "Msg from client fd " << sockFd << " : " << readBuffer->c_str() << '\n';
            send(sockFd);
            readBuffer->clear();
            break;
        }
        else
        {
            std::cout << "Connection reset by peer.\n";
            deleteConnectionCallback(sockFd);
            break;
        }
    }
}

void Connection::send(int sockFd)
{
    char buf[readBuffer->size()];
    strcpy(buf, readBuffer->c_str());
    int dataSize = readBuffer->size();
    int dataLeft = dataSize;
    while (dataLeft > 0)
    {
        const ssize_t writeBytes = write(sockFd, buf + dataSize - dataLeft, dataLeft);
        if (writeBytes == -1 && errno == EAGAIN)
            break;
        dataLeft -= writeBytes;
    }
}

void Connection::setDeleteConnectionCallback(std::function<void(int)> _cb)
{
    deleteConnectionCallback = _cb;
}
