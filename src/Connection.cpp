#include "Connection.hpp"

#include "Buffer.hpp"
#include "Channel.hpp"
#include "EventLoop.hpp"
#include "Macros.hpp"
#include "Socket.hpp"

#include <cassert>
#include <cerrno>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <utility>

Connection::Connection(int fd, EventLoop *loop)
{
    socket_ = std::make_unique<Socket>();
    socket_->setFd(fd);
    if (loop != nullptr)
    {
        channel_ = std::make_unique<Channel>(fd, loop);
        channel_->enableRead();
        channel_->enableET();
    }
    readBuffer_ = std::make_unique<Buffer>();
    sendBuffer_ = std::make_unique<Buffer>();

    state_ = State::Connected;
}

Connection::~Connection()
{
}

RC Connection::read()
{
    if (state_ != State::Connected)
    {
        perror("Connection is not onnected, can not read");
        return RC_CONNECTION_ERROR;
    }
    assert(state_ == State::Connected && "Connection state is disconnected.");
    readBuffer_->clear();
    if (socket_->isNonBlocking())
        return readNonBlocking();
    else
        return readBlocking();
}

RC Connection::write()
{
    if (state_ != State::Connected)
    {
        perror("Connection is not onnected, can not write");
        return RC_CONNECTION_ERROR;
    }
    RC rc = RC_UNDEFINED;
    if (socket_->isNonBlocking())
        rc = writeNonBlocking();
    else
        rc = writeBlocking();
    sendBuffer_->clear();
    return rc;
}

RC Connection::send(const std::string &msg)
{
    setSendBuffer(msg.c_str());
    write();
    return RC_SUCCESS;
}

void Connection::business()
{
    read();
    onRecvCallback(this);
}

RC Connection::readNonBlocking()
{
    int sockfd = socket_->getFd();
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
            std::cout << "continue reading\n";
            continue;
        }
        else if (readBytes == -1 && ((errno == EAGAIN) || (errno == EWOULDBLOCK)))
        {
            break;
        }
        else if (readBytes == 0)
        {
            std::cout << "read EOF, client fd " << sockfd << " disconnected\n";
            state_ = State::Closed;
            close();
            break;
        }
        else
        {
            std::cout << "Other error on client fd " << sockfd << '\n';
            state_ = State::Closed;
            close();
            break;
        }
    }
    return RC_SUCCESS;
}

RC Connection::writeNonBlocking()
{
    int sockfd = socket_->getFd();
    char buf[sendBuffer_->size()];
    memcpy(buf, sendBuffer_->c_str(), sendBuffer_->size());
    int data_size = sendBuffer_->size();
    int data_left = data_size;
    while (data_left > 0)
    {
        ssize_t bytes_write = ::write(sockfd, buf + data_size - data_left, data_left);
        if (bytes_write == -1 && errno == EINTR)
        {
            std::cout << "continue writing\n";
            continue;
        }
        if (bytes_write == -1 && errno == EAGAIN)
        {
            break;
        }
        if (bytes_write == -1)
        {
            std::cout << "Other error on client fd " << sockfd << '\n';
            state_ = State::Closed;
            break;
        }
        data_left -= bytes_write;
    }
    return RC_SUCCESS;
}

// Never used by server, only for client
RC Connection::readBlocking()
{
    int sockfd = socket_->getFd();
    size_t data_size = socket_->recvBufSize();
    char buf[1024];
    ssize_t bytes_read = ::read(sockfd, buf, sizeof(buf));
    if (bytes_read > 0)
    {
        readBuffer_->append(buf, bytes_read);
    }
    else if (bytes_read == 0)
    {
        std::cout << "read EOF, blocking client fd " << sockfd << " disconnected\n";
        state_ = State::Closed;
    }
    else if (bytes_read == -1)
    {
        std::cout << "Other error on blocking client fd " << sockfd << '\n';
        state_ = State::Closed;
    }
    return RC_SUCCESS;
}

// Never used by server, only for client
RC Connection::writeBlocking()
{
    int sockfd = socket_->getFd();
    ssize_t bytes_write = ::write(sockfd, sendBuffer_->c_str(), sendBuffer_->size());
    if (bytes_write == -1)
    {
        std::cout << "Other error on blocking client fd " << sockfd << '\n';
        state_ = State::Closed;
    }
    return RC_SUCCESS;
}

void Connection::close()
{
    deleteConnectionCallback(socket_->getFd());
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
    return readBuffer_.get();
}

Buffer *Connection::getSendBuffer() const
{
    return sendBuffer_.get();
}

void Connection::setDeleteConnectionCallback(std::function<void(int)> const &callback)
{
    deleteConnectionCallback = callback;
}

void Connection::setDeleteConnectionCallback(std::function<void(int)> &&callback)
{
    deleteConnectionCallback = std::move(callback);
}

void Connection::setOnRecvCallback(std::function<void(Connection *)> const &callback)
{
    onRecvCallback = callback;
    std::function<void()> bus = std::bind(&Connection::business, this);
    channel_->setReadCallback(bus);
}

void Connection::setOnRecvCallback(std::function<void(Connection *)> &&callback)
{
    onRecvCallback = std::move(callback);
    std::function<void()> bus = std::bind(&Connection::business, this);
    channel_->setReadCallback(bus);
}

Socket *Connection::getSocket() const
{
    return socket_.get();
}
