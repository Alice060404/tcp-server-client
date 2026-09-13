#include <cstdint>
#include <sys/epoll.h>

#include "../include/channel.hpp"
#include "../include/epoll.hpp"

Channel::Channel(Epoll *_ep, int _fd) : ep(_ep), fd(_fd), events(0), revents(0), inEpoll(false)
{
}

Channel::~Channel()
{
}

void Channel::enableReading()
{
    events = EPOLLIN | EPOLLET;
    ep->updateChannel(this);
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
