#pragma once

#include <cstddef>
#include <netinet/in.h>
#include <sys/socket.h>

class InetAddress
{
  public:
    sockaddr_in addr;
    socklen_t addrLen;

    InetAddress();
    InetAddress(const char *ip, std::size_t port);
    ~InetAddress();

    sockaddr_in getAddr() const;
    socklen_t getAddrLen() const;
};
