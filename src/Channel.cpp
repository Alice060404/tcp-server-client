#include <cstdint>
#include <functional>
#include <sys/epoll.h>
#include <unistd.h>

#include "Channel.hpp"
#include "Epoll.hpp"
#include "EventLoop.hpp"

Channel::Channel(EventLoop *_loop, int _fd) : loop(_loop), fd(_fd), events(0), revents(0), inEpoll(false)
{
}

Channel::~Channel()
{
    if (fd != -1)
    {
        close(fd);
        fd = -1;
    }
}

void Channel::enableReading()
{
    events |= EPOLLIN | EPOLLPRI;
    loop->updateChannel(this);
}

void Channel::useET()
{
    events |= EPOLLET;
    loop->updateChannel(this);
}

int Channel::getFd() const
{
    return fd;
}

uint32_t Channel::getListenEvents() const
{
    return events;
}

uint32_t Channel::getReadyEvents() const
{
    return revents;
}

bool Channel::getInepoll() const
{
    return inEpoll;
}

void Channel::setInepoll(bool _in)
{
    inEpoll = _in;
}

void Channel::setReadyEvents(uint32_t _ev)
{
    revents = _ev;
}

void Channel::handleEvent()
{
    if (revents & (EPOLLIN | EPOLLPRI))
    {
        readCallback();
    }
    if (revents & (EPOLLOUT))
    {
        writeCallback();
    }
}

void Channel::setReadCallback(std::function<void()> const &_callback)
{
    readCallback = _callback;
}
