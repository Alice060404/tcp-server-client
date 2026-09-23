#pragma once

#include "Macros.hpp"
class Epoll;
class Channel;
class ThreadPool;

class EventLoop
{
  private:
    Epoll *ep{nullptr};
    bool quit{false};

  public:
    EventLoop();
    ~EventLoop();

    DISALLOW_COPY_AND_MOVE(EventLoop)

    void Loop();

    void updateChannel(Channel *channel);
};
