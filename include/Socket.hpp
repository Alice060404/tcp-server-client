#pragma once

#include "Macros.hpp"
#include <cstdint>

class InetAddress;

class Socket
{
  private:
    int fd{-1};

  public:
    Socket();
    explicit Socket(int _fd);
    ~Socket();

    DISALLOW_COPY_AND_MOVE(Socket)

    void bind(InetAddress *addr);
    void listen();
    void setNonBlocking();
    bool isNonBlocking() const;

    int accept(InetAddress *addr);
    void connect(InetAddress *addr);
    void connect(const char *IP, uint16_t PORT);

    int getFd() const;
};
