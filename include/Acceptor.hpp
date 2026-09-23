#pragma once

#include <functional>

#include "Macros.hpp"

class EventLoop;
class Socket;
class Channel;
class InetAddress;

class Acceptor
{
  private:
    EventLoop *loop;
    Socket *sock;
    Channel *acceptChannel;
    std::function<void(Socket *)> newConnectionCallback;

  public:
    explicit Acceptor(EventLoop *_loop);
    ~Acceptor();

    DISALLOW_COPY_AND_MOVE(Acceptor)

    void setNewConnectionCallback(std::function<void(Socket *)> const &_newConnectionCallback);
    void acceptConnection();
};
