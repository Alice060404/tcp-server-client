#include "Socket.hpp"

#include "Macros.hpp"

#include <arpa/inet.h>
#include <asm-generic/ioctls.h>
#include <cassert>
#include <cerrno>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <netinet/in.h>
#include <string>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

Socket::Socket() : fd_(-1)
{
}

Socket::~Socket()
{
    if (fd_ != -1)
    {
        close(fd_);
        fd_ = -1;
    }
}

void Socket::setFd(int fd)
{
    fd_ = fd;
}

std::string Socket::getAddr() const
{
    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    socklen_t len = sizeof(addr);
    if (getpeername(fd_, (sockaddr *)&addr, &len) == -1)
        return "";
    std::string ret(inet_ntoa(addr.sin_addr));
    ret += ":";
    ret += std::to_string(htons(addr.sin_port));
    return ret;
}

RC Socket::bind(const char *ip, uint16_t port)
{
    assert(fd_ != -1);
    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(ip);
    addr.sin_port = htons(port);
    if (::bind(fd_, (sockaddr *)&addr, sizeof(addr)) == -1)
    {
        perror("failed to bind socket");
        return RC_SOCKET_ERROR;
    }
    return RC_SUCCESS;
}

RC Socket::listen() const
{
    assert(fd_ != -1);
    if (::listen(fd_, SOMAXCONN) == -1)
    {
        perror("failed to listen socket");
        return RC_SOCKET_ERROR;
    }
    return RC_SUCCESS;
}

RC Socket::setNonBlocking() const
{
    if (fcntl(fd_, F_SETFL, fcntl(fd_, F_GETFL) | O_NONBLOCK) == -1)
    {
        perror("Socket set non-blocking failed");
        return RC_SOCKET_ERROR;
    }
    return RC_SUCCESS;
}

bool Socket::isNonBlocking() const
{
    return (fcntl(fd_, F_GETFL) & O_NONBLOCK) != 0;
}

size_t Socket::recvBufSize() const
{
    size_t size = -1;
    if (ioctl(fd_, FIONREAD, &size) == -1)
        perror("Socket get recv buf size failed");
    return size;
}

RC Socket::create()
{
    assert(fd_ == -1);
    fd_ = ::socket(AF_INET, SOCK_STREAM, 0);
    if (fd_ == -1)
    {
        perror("failed to create socket");
        return RC_SOCKET_ERROR;
    }
    return RC_SUCCESS;
}

RC Socket::accept(int &clntFd) const
{
    assert(fd_ != -1);
    clntFd = ::accept(fd_, nullptr, nullptr);
    if (clntFd == -1)
    {
        perror("failed to accept socket");
        return RC_SOCKET_ERROR;
    }
    return RC_SUCCESS;
}

RC Socket::connect(const char *ip, uint16_t port) const
{
    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(ip);
    addr.sin_port = htons(port);
    if (::connect(fd_, (struct sockaddr *)&addr, sizeof(addr)) == -1)
    {
        perror("Failed to connect socket");
        return RC_SOCKET_ERROR;
    }
    return RC_SUCCESS;
}

int Socket::getFd() const
{
    return fd_;
}
