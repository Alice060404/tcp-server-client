#include <fcntl.h>
#include <sys/socket.h>
#include <unistd.h>

#include "../include/InetAddress.hpp"
#include "../include/common.hpp"
#include "../include/Socket.hpp"

Socket::Socket() : fd(-1)
{
    fd = ::socket(AF_INET, SOCK_STREAM, 0);
    common::exception::throw_if(fd == -1, "Failed to sock.");
}

Socket::Socket(int _fd) : fd(_fd)
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
    common::exception::throw_if(::bind(fd, reinterpret_cast<sockaddr *>(&addr->addr), addr->addrLen) == -1,
                                "Failed to bind.");
}

void Socket::listen()
{
    common::exception::throw_if(::listen(fd, SOMAXCONN) == -1, "Failed to listen.");
}

int Socket::accept(InetAddress *addr)
{
    int clientSockFd = ::accept(fd, reinterpret_cast<sockaddr *>(&addr->addr), &addr->addrLen);
    common::exception::throw_if(clientSockFd == -1, "Sock accept error.");
    return clientSockFd;
}

void Socket::setnonblocking()
{
    fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);
}

int Socket::getFd() const
{
    return fd;
}
