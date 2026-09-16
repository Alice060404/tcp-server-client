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
    InetAddress *addr;
    Channel *acceptChannel;

  public:
    Acceptor(EventLoop *_loop);
    ~Acceptor();

    std::function<void(Socket *)> newConnectionCallback;

    void setNewConnectionCallback(std::function<void(Socket *_cb)> _newConnectionCallback);
    void acceptConnection();
};
