#pragma once

#include "Macros.hpp"
#include <cstdint>
#include <netinet/in.h>
#include <sys/socket.h>

class InetAddress
{
  private:
    sockaddr_in addr{};

  public:
    InetAddress();
    InetAddress(const char *ip, uint16_t port);
    ~InetAddress() = default;

    DISALLOW_COPY_AND_MOVE(InetAddress)

    void setAddr(sockaddr_in _addr);
    sockaddr_in getAddr() const;
    const char *getIP() const;
    uint16_t getPort() const;
};
