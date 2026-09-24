#include "Connection.hpp"

#include "Buffer.hpp"
#include "Channel.hpp"
#include "Macros.hpp"
#include "Socket.hpp"

#include <cassert>
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <functional>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

Connection::Connection(EventLoop *eventLoop, Socket *socket) : loop(eventLoop), sock(socket)
{
    if (loop != nullptr)
    {
        channel = new Channel(loop, sock);
        channel->enableRead();
        channel->useET();
    }
    readBuffer_ = new Buffer();
    sendBuffer_ = new Buffer();
    state_ = State::Connected;
}

Connection::~Connection()
{
    if (loop != nullptr)
    {
        delete channel;
    }
    delete sock;
    delete readBuffer_;
    delete sendBuffer_;
}

void Connection::read()
{
    ASSERT(state_ == State::Connected, "Connection state is disconnected.");
    readBuffer_->clear();
    if (sock->isNonBlocking())
        readNonBlocking();
    else
        readBlocking();
}

void Connection::write()
{
    ASSERT(state_ == State::Connected, "Connection state is disconnected.");
    if (sock->isNonBlocking())
        writeNonBlocking();
    else
        writeBlocking();
    sendBuffer_->clear();
}

void Connection::send(const std::string &msg)
{
    setSendBuffer(msg.c_str());
    write();
}

void Connection::business()
{
    read();
    onMessageCallback(this);
}

void Connection::readNonBlocking()
{
    int sockfd = sock->getFd();
    char buf[1024];
    while (true)
    {
        memset(buf, 0, sizeof(buf));
        ssize_t readBytes = ::read(sockfd, buf, sizeof(buf));
        if (readBytes > 0)
        {
            readBuffer_->append(buf, readBytes);
        }
        else if (readBytes == -1 && errno == EINTR)
        {
            printf("continue reading\n");
            continue;
        }
        else if (readBytes == -1 && ((errno == EAGAIN) || (errno == EWOULDBLOCK)))
        {
            break;
        }
        else if (readBytes == 0)
        {
            printf("read EOF, client fd %d disconnected\n", sockfd);
            state_ = State::Closed;
            close();
            break;
        }
        else
        {
            printf("Other error on client fd %d\n", sockfd);
            state_ = State::Closed;
            close();
            break;
        }
    }
}

void Connection::writeNonBlocking()
{
    int sockfd = sock->getFd();
    char buf[sendBuffer_->size()];
    memcpy(buf, sendBuffer_->c_str(), sendBuffer_->size());
    int data_size = sendBuffer_->size();
    int data_left = data_size;
    while (data_left > 0)
    {
        ssize_t bytes_write = ::write(sockfd, buf + data_size - data_left, data_left);
        if (bytes_write == -1 && errno == EINTR)
        {
            printf("continue writing\n");
            continue;
        }
        if (bytes_write == -1 && errno == EAGAIN)
        {
            break;
        }
        if (bytes_write == -1)
        {
            printf("Other error on client fd %d\n", sockfd);
            state_ = State::Closed;
            break;
        }
        data_left -= bytes_write;
    }
}

// Never used by server, only for client
void Connection::readBlocking()
{
    int sockfd = sock->getFd();
    unsigned int rcv_size = 0;
    socklen_t len = sizeof(rcv_size);
    getsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, &rcv_size, &len);
    char buf[rcv_size];
    ssize_t bytes_read = ::read(sockfd, buf, sizeof(buf));
    if (bytes_read > 0)
    {
        readBuffer_->append(buf, bytes_read);
    }
    else if (bytes_read == 0)
    {
        printf("read EOF, blocking client fd %d disconnected\n", sockfd);
        state_ = State::Closed;
    }
    else if (bytes_read == -1)
    {
        printf("Other error on blocking client fd %d\n", sockfd);
        state_ = State::Closed;
    }
}

// Never used by server, only for client
void Connection::writeBlocking()
{
    int sockfd = sock->getFd();
    ssize_t bytes_write = ::write(sockfd, sendBuffer_->c_str(), sendBuffer_->size());
    if (bytes_write == -1)
    {
        printf("Other error on blocking client fd %d\n", sockfd);
        state_ = State::Closed;
    }
}

void Connection::close()
{
    deleteConnectionCallback(sock);
}

Connection::State Connection::getState() const
{
    return state_;
}

void Connection::setSendBuffer(const char *str)
{
    sendBuffer_->setBuf(str);
}

Buffer *Connection::getReadBuffer() const
{
    return readBuffer_;
}

const char *Connection::readBuffer() const
{
    return readBuffer_->c_str();
}

Buffer *Connection::getSendBuffer() const
{
    return sendBuffer_;
}

const char *Connection::sendBuffer() const
{
    return sendBuffer_->c_str();
}

void Connection::setDeleteConnectionCallback(std::function<void(Socket *)> const &callback)
{
    deleteConnectionCallback = callback;
}

void Connection::setOnConnectCallback(std::function<void(Connection *)> const &callback)
{
    onConnectCallback = callback;
    channel->setReadCallback([this]() { onConnectCallback(this); });
}

void Connection::setOnMessageCallback(std::function<void(Connection *)> const &callback)
{
    onMessageCallback = callback;
    std::function<void()> bus = std::bind(&Connection::business, this);
    channel->setReadCallback(bus);
}

void Connection::getlineSendBuffer()
{
    sendBuffer_->getline();
}

Socket *Connection::getSocket() const
{
    return sock;
}
