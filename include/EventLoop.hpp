#pragma once

#include "Macros.hpp"

#include <memory>

class Poller;
class Channel;

class EventLoop
{
  public:
    EventLoop();
    ~EventLoop();

    DISALLOW_COPY_AND_MOVE(EventLoop)

    void Loop() const;
    void updateChannel(Channel *channel) const;
    void deleteChannel(Channel *channel) const;

  private:
    std::unique_ptr<Poller> poller_;
};
