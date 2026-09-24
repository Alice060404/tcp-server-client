#pragma once

#include "Macros.hpp"

#include <vector>

#ifdef OS_LINUX
#include <sys/epoll.h>
#endif

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

    void deleteChannel(Channel *channel);
    void updateChannel(Channel *channel);

    std::vector<Channel *> poll(int timeout = -1);

  private:
    int fd{1};
#ifdef OS_LINUX
    epoll_event *events{nullptr};
#endif

#ifdef OS_MACOS
    kevent *events{nullptr};
#endif
};
