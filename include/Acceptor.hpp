#pragma once

#include "Macros.hpp"

#include <functional>
#include <memory>

class EventLoop;
class Socket;
class Channel;

class Acceptor
{
  public:
    explicit Acceptor(EventLoop *eventLoop);
    ~Acceptor();

    DISALLOW_COPY_AND_MOVE(Acceptor)

    void setNewConnectionCallback(std::function<void(int)> const &callback);
    RC acceptConnection() const;

  private:
    std::unique_ptr<Socket> socket_;
    std::unique_ptr<Channel> channel_;
    std::function<void(int)> newConnectionCallback_;
};
