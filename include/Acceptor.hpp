#pragma once

#include <functional>

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
    Acceptor(EventLoop *_loop);
    ~Acceptor();

    void setNewConnectionCallback(std::function<void(Socket *)> _newConnectionCallback);
    void acceptConnection();
};
