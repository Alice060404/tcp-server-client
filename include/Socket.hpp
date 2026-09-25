#pragma once

#include "Macros.hpp"

#include <cstddef>
#include <cstdint>
#include <string>

class Socket
{
  public:
    Socket();
    ~Socket();

    DISALLOW_COPY_AND_MOVE(Socket)

    void setFd(int fd);

    RC create();
    RC bind(const char *ip, uint16_t port);
    RC listen() const;
    RC setNonBlocking() const;
    bool isNonBlocking() const;

    RC accept(int &clntFd) const;
    RC connect(const char *ip, uint16_t port) const;

    size_t recvBufSize() const;

    int getFd() const;
    std::string getAddr() const;

  private:
    int fd_;
};
