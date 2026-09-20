#pragma once

#include <cstddef>
#include <cstdint>
#include <netinet/in.h>
#include <sys/socket.h>

class InetAddress
{
  public:
    sockaddr_in addr;
    socklen_t addrLen;

    InetAddress();
    InetAddress(const char *ip, uint16_t port);
    ~InetAddress();

    sockaddr_in getAddr() const;
    socklen_t getAddrLen() const;
};
