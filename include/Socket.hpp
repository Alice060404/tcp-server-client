#pragma once

#include "Macros.hpp"

#include <cstdint>

class InetAddress;

class Socket
{
  public:
    Socket();
    explicit Socket(int socketFd);
    ~Socket();

    DISALLOW_COPY_AND_MOVE(Socket)

    void bind(InetAddress *addr);
    void listen();
    void setNonBlocking();
    bool isNonBlocking() const;

    int accept(InetAddress *addr);
    void connect(InetAddress *addr);
    void connect(const char *ip, uint16_t port);

    int getFd() const;

  private:
    int fd{-1};
};
