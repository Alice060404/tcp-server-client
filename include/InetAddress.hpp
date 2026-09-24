#pragma once

#include "Macros.hpp"

#include <cstdint>
#include <netinet/in.h>

class InetAddress
{
  public:
    InetAddress();
    InetAddress(const char *ip, uint16_t port);
    ~InetAddress() = default;

    DISALLOW_COPY_AND_MOVE(InetAddress)

    void setAddr(sockaddr_in address);
    sockaddr_in getAddr() const;
    const char *getIP() const;
    uint16_t getPort() const;

  private:
    sockaddr_in addr{};
};
