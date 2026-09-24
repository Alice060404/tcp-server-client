#include "Socket.hpp"

#include "InetAddress.hpp"
#include "common.hpp"

#include <cerrno>
#include <cstdint>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

Socket::Socket()
{
    fd = ::socket(AF_INET, SOCK_STREAM, 0);
    common::exception::throw_if(fd == -1, "Failed to sock.");
}

Socket::Socket(int socketFd) : fd(socketFd)
{
    common::exception::throw_if(fd == -1, "Failed to sock.");
}

Socket::~Socket()
{
    if (fd != -1)
    {
        close(fd);
        fd = -1;
    }
}

void Socket::bind(InetAddress *addr)
{
    sockaddr_in tmpAddr = addr->getAddr();
    common::exception::throw_if(::bind(fd, (sockaddr *)&tmpAddr, sizeof(tmpAddr)) == -1, "Failed to bind.");
}

void Socket::listen()
{
    common::exception::throw_if(::listen(fd, SOMAXCONN) == -1, "Failed to listen.");
}

void Socket::setNonBlocking()
{
    fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);
}

bool Socket::isNonBlocking() const
{
    return (fcntl(fd, F_GETFL) & O_NONBLOCK) != 0;
}

int Socket::accept(InetAddress *addr)
{
    // for server sock
    int clntSockFd = -1;
    sockaddr_in tmpAddr{};
    socklen_t addrLen = sizeof(tmpAddr);
    if (fcntl(fd, F_GETFL) & O_NONBLOCK)
    {
        while (true)
        {
            clntSockFd = ::accept(fd, (sockaddr *)&tmpAddr, &addrLen);
            if (clntSockFd == -1 && ((errno == EAGAIN) || errno == EWOULDBLOCK))
            {
                continue;
            }
            if (clntSockFd == -1)
            {
                common::exception::throw_if(true, "Sock accept error.");
            }
            else
                break;
        }
    }
    else
    {
        clntSockFd = ::accept(fd, (sockaddr *)&tmpAddr, &addrLen);
        common::exception::throw_if(clntSockFd == -1, "Sock accept error.");
    }
    addr->setAddr(tmpAddr);
    return clntSockFd;
}

void Socket::connect(InetAddress *addr)
{
    // for client sock
    sockaddr_in tmpAddr = addr->getAddr();
    if (fcntl(fd, F_GETFL) & O_NONBLOCK)
    {
        while (true)
        {
            int ret = ::connect(fd, (sockaddr *)&tmpAddr, sizeof(tmpAddr));
            if (ret == 0)
                break;
            if (ret == -1 && errno == EINPROGRESS)
                continue;
            if (ret == -1)
                common::exception::throw_if(true, "Sock connect error.");
        }
    }
    else
        common::exception::throw_if(::connect(fd, (sockaddr *)&tmpAddr, sizeof(tmpAddr)) == -1, "Sock connect error.");
}

void Socket::connect(const char *ip, uint16_t port)
{
    InetAddress *addr = new InetAddress(ip, port);
    connect(addr);
    delete addr;
}

int Socket::getFd() const
{
    return fd;
}
