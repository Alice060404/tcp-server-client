#include "Poller.hpp"

#include "Channel.hpp"
#include "Socket.hpp"
#include "common.hpp"

#include <cstddef>
#include <cstring>
#include <sys/epoll.h>
#include <unistd.h>
#include <vector>

constexpr std::size_t MAX_EVENTS = 1000;

#ifdef OS_LINUX

Poller::Poller()
{
    fd = epoll_create1(0);
    common::exception::throw_if(fd == -1, "Epoll create error");
    events = new epoll_event[MAX_EVENTS];
    memset(events, 0, sizeof(*events) * MAX_EVENTS);
}

Poller::~Poller()
{
    if (fd != -1)
    {
        close(fd);
    }
    delete[] events;
}

std::vector<Channel *> Poller::poll(int timeout)
{
    std::vector<Channel *> active_channels;
    int nfds = epoll_wait(fd, events, MAX_EVENTS, timeout);
    common::exception::throw_if(nfds == -1, "epoll wait error");
    for (int i = 0; i < nfds; ++i)
    {
        Channel *ch = (Channel *)events[i].data.ptr;
        int events_ = events[i].events;
        if (events_ & EPOLLIN)
        {
            ch->setReadyEvents(Channel::READ_EVENT);
        }
        if (events_ & EPOLLOUT)
        {
            ch->setReadyEvents(Channel::WRITE_EVENT);
        }
        if (events_ & EPOLLET)
        {
            ch->setReadyEvents(Channel::ET);
        }
        active_channels.push_back(ch);
    }
    return active_channels;
}

void Poller::updateChannel(Channel *ch)
{
    int sockfd = ch->getSocket()->getFd();
    struct epoll_event ev
    {
    };
    ev.data.ptr = ch;
    if (ch->getListenEvents() & Channel::READ_EVENT)
    {
        ev.events |= EPOLLIN | EPOLLPRI;
    }
    if (ch->getListenEvents() & Channel::WRITE_EVENT)
    {
        ev.events |= EPOLLOUT;
    }
    if (ch->getListenEvents() & Channel::ET)
    {
        ev.events |= EPOLLET;
    }
    if (!ch->getExist())
    {
        common::exception::throw_if(epoll_ctl(fd, EPOLL_CTL_ADD, sockfd, &ev) == -1, "epoll add error");
        ch->setExist();
    }
    else
    {
        common::exception::throw_if(epoll_ctl(fd, EPOLL_CTL_MOD, sockfd, &ev) == -1, "epoll modify error");
    }
}

void Poller::deleteChannel(Channel *ch)
{
    int sockfd = ch->getSocket()->getFd();
    common::exception::throw_if(epoll_ctl(fd, EPOLL_CTL_DEL, sockfd, nullptr) == -1, "epoll delete error");
    ch->setExist(false);
}

#endif

#ifdef OS_MACOS

Poller::Poller()
{
    fd = kqueue();
    common::exception::throw_if(fd == -1, "Kqueue create error");
    events = new struct kevent[MAX_EVENTS];
    memset(events, 0, sizeof(*events) * MAX_EVENTS);
}

Poller::~Poller()
{
    if (fd != -1)
    {
        close(fd);
    }
}

std::vector<Channel *> Poller::poll(int timeout)
{
    std::vector<Channel *> active_channels;
    struct timespec ts;
    memset(&ts, 0, sizeof(ts));
    if (timeout != -1)
    {
        ts.tv_sec = timeout / 1000;
        ts.tv_nsec = (timeout % 1000) * 1000 * 1000;
    }
    int nfds = 0;
    if (timeout == -1)
    {
        nfds = kevent(fd, NULL, 0, events, MAX_EVENTS, NULL);
    }
    else
    {
        nfds = kevent(fd, NULL, 0, events, MAX_EVENTS, &ts);
    }
    for (int i = 0; i < nfds; ++i)
    {
        Channel *ch = (Channel *)events[i].udata;
        int events_ = events[i].filter;
        if (events_ == EVFILT_READ)
        {
            ch->setReadyEvents(ch->READ_EVENT | ch->ET);
        }
        if (events_ == EVFILT_WRITE)
        {
            ch->setReadyEvents(ch->WRITE_EVENT | ch->ET);
        }
        active_channels.push_back(ch);
    }
    return active_channels;
}

void Poller::updateChannel(Channel *ch)
{
    struct kevent ev[2];
    memset(ev, 0, sizeof(*ev) * 2);
    int n = 0;
    int fd = ch->getSocket()->getFd();
    int op = EV_ADD;
    if (ch->getListenEvents() & ch->ET)
    {
        op |= EV_CLEAR;
    }
    if (ch->getListenEvents() & ch->READ_EVENT)
    {
        EV_SET(&ev[n++], fd, EVFILT_READ, op, 0, 0, ch);
    }
    if (ch->getListenEvents() & ch->WRITE_EVENT)
    {
        EV_SET(&ev[n++], fd, EVFILT_WRITE, op, 0, 0, ch);
    }
    int r = kevent(fd_, ev, n, NULL, 0, NULL);
    common::exception::throw_if(r == -1, "kqueue add event error");
}

void Poller::deleteChannel(Channel *ch)
{
    struct kevent ev[2];
    int n = 0;
    int fd_ = ch->getSocket()->getFd();
    if (ch->getListenEvents() & ch->READ_EVENT)
    {
        EV_SET(&ev[n++], fd_, EVFILT_READ, EV_DELETE, 0, 0, ch);
    }
    if (ch->getListenEvents() & ch->WRITE_EVENT)
    {
        EV_SET(&ev[n++], fd_, EVFILT_WRITE, EV_DELETE, 0, 0, ch);
    }
    int r = kevent(fd, ev, n, NULL, 0, NULL);
    common::exception::throw_if(r == -1, "kqueue delete event error");
}
#endif
