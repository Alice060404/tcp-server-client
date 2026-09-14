#include "../include/Epoll.hpp"
#include <cstddef>
#include <cstdint>
#include <netinet/in.h>
#include <strings.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <vector>

#include "../include/Channel.hpp"
#include "../include/common.hpp"

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

void Epoll::updateChannel(Channel *channel)
{
    int ChannelFd = channel->getFd();
    epoll_event ev;
    bzero(&ev, sizeof(ev));
    ev.data.ptr = channel;
    ev.events = channel->getEvents();

    if (!channel->getInepoll())
    {
        common::exception::throw_if(epoll_ctl(epollFd, EPOLL_CTL_ADD, ChannelFd, &ev) == -1, "Epoll add error.");
        channel->setInepoll();
    }
    else
    {
        common::exception::throw_if(epoll_ctl(epollFd, EPOLL_CTL_MOD, ChannelFd, &ev) == -1, "Epoll modify error.");
    }
}

std::vector<Channel *> Epoll::poll(int timeout)
{
    std::vector<Channel *> activeEvents;
    int nfds = epoll_wait(epollFd, events, MAX_EVENTS, timeout);
    common::exception::throw_if(nfds == -1, "Epoll wait error");
    for (int i = 0; i < nfds; ++i)
    {
        Channel *channel = (Channel *)events[i].data.ptr;
        channel->setRevents(events[i].events);
        activeEvents.push_back(channel);
    }
    return activeEvents;
}
