#pragma once

#include "Macros.hpp"

class Poller;
class Channel;

class EventLoop
{
  public:
    EventLoop();
    ~EventLoop();

    DISALLOW_COPY_AND_MOVE(EventLoop)

    void Loop();
    void updateChannel(Channel *channel);
    void deleteChannel(Channel *channel);
    void quit();

  private:
    Poller *poller_{nullptr};
    bool quit_{false};
};
