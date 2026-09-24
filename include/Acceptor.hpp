#pragma once

#include "Macros.hpp"

#include <functional>

class EventLoop;
class Socket;
class Channel;

class Acceptor
{
  public:
    explicit Acceptor(EventLoop *eventLoop);
    ~Acceptor();

    DISALLOW_COPY_AND_MOVE(Acceptor)

    void setNewConnectionCallback(std::function<void(Socket *)> const &callback);
    void acceptConnection();

  private:
    EventLoop *loop;
    Socket *sock;
    Channel *acceptChannel;
    std::function<void(Socket *)> newConnectionCallback;
};
