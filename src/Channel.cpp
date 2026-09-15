#include <cstdint>
#include <functional>
#include <sys/epoll.h>

#include "../include/Channel.hpp"
#include "../include/Epoll.hpp"
#include "../include/EventLoop.hpp"

Channel::Channel(EventLoop *_loop, int _fd) : loop(_loop), fd(_fd), events(0), revents(0), inEpoll(false)
{
}

Channel::~Channel()
{
}

void Channel::enableReading()
{
    events = EPOLLIN | EPOLLET;
    loop->updateChannel(this);
}

int Channel::getFd() const
{
    return fd;
}

uint32_t Channel::getEvents() const
{
    return events;
}

uint32_t Channel::getRevents() const
{
    return revents;
}

bool Channel::getInepoll() const
{
    return inEpoll;
}

void Channel::setInepoll()
{
    inEpoll = true;
}

void Channel::setRevents(uint32_t _ev)
{
    revents = _ev;
}

void Channel::handleEvent()
{
    callback();
}

void Channel::setCallback(std::function<void()> _cb)
{
    callback = _cb;
}
