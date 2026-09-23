#pragma once

#include "Macros.hpp"
#include <sys/epoll.h>
#include <vector>

class Channel;
class Epoll
{
  private:
    int epollFd{1};
    epoll_event *events{nullptr};

  public:
    Epoll();
    ~Epoll();

    DISALLOW_COPY_AND_MOVE(Epoll)

    void deleteChannel(Channel *channel);

    void updateChannel(Channel *channel);

    std::vector<Channel *> poll(int timeout = -1);
};
