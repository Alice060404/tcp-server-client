#include "../include/epoll.hpp"
#include <cstddef>
#include <cstdint>
#include <strings.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <vector>

#include "common.cpp"

constexpr std::size_t MAX_EVENTS = 1024;

Epoll::Epoll() : epollFd(-1), events(nullptr)
{
    epollFd = epoll_create1(0);
    common::exception::throw_if(epollFd == -1, "Epoll create error.");

    events = new epoll_event[MAX_EVENTS];
    bzero(events, sizeof(*events) * MAX_EVENTS);
}

Epoll::~Epoll()
{
    if (epollFd != -1)
    {
        close(epollFd);
        epollFd = -1;
    }
    delete[] events;
}

void Epoll::addFd(int fd, uint32_t op)
{
    epoll_event ev;
    bzero(&ev, sizeof(ev));

    ev.data.fd = fd;
    ev.events = op;
    common::exception::throw_if(epoll_ctl(epollFd, EPOLL_CTL_ADD, fd, &ev) == -1, "epoll add event error.");
}

std::vector<epoll_event> Epoll::poll(int timeout)
{
    std::vector<epoll_event> activeEvents;
    int nfds = epoll_wait(epollFd, events, MAX_EVENTS, timeout);
    common::exception::throw_if(nfds == -1, "Epoll wait error");
    for (int i = 0; i < nfds; ++i)
    {
        activeEvents.push_back(events[i]);
    }
    return activeEvents;
}
