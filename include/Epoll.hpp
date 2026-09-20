#pragma once

#include <cstdint>
#include <sys/epoll.h>
#include <vector>

class Channel;
class Epoll
{
  private:
    int epollFd;
    epoll_event *events;

  public:
    Epoll();

    ~Epoll();

    void deleteChannel(Channel *channel);

    void updateChannel(Channel *channel);

    std::vector<Channel *> poll(int timeout = -1);
};
