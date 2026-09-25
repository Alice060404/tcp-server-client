#pragma once

#include "Macros.hpp"

#include <sys/epoll.h>
#include <vector>

#ifdef OS_MACOS
#include <sys/event.h>
#endif

class Channel;
class Poller
{
  public:
    Poller();
    ~Poller();

    DISALLOW_COPY_AND_MOVE(Poller)

    RC deleteChannel(Channel *channel) const;
    RC updateChannel(Channel *channel) const;

    std::vector<Channel *> poll(int timeout = -1) const;

  private:
    int fd_;
    epoll_event *events_{nullptr};

#ifdef OS_MACOS
    kevent *events_{nullptr};
#endif
};
