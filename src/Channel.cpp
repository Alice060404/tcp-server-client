#include <cstdint>
#include <functional>
#include <sys/epoll.h>
#include <unistd.h>

#include "Channel.hpp"
#include "Epoll.hpp"
#include "EventLoop.hpp"

Channel::Channel(EventLoop *_loop, int _fd)
    : loop(_loop), fd(_fd), events(0), revents(0), inEpoll(false), useThreadPool(true)
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

void Channel::setInepoll(bool _in)
{
    inEpoll = _in;
}

void Channel::setRevents(uint32_t _ev)
{
    revents = _ev;
}

void Channel::handleEvent()
{
    if (revents & (EPOLLIN | EPOLLPRI))
    {
        if (useThreadPool)
            loop->addThread(readCallback);
        else
            readCallback();
    }
    if (revents & (EPOLLOUT))
    {
        if (useThreadPool)
            loop->addThread(writeCallback);
        else
            writeCallback();
    }
}

void Channel::setReadCallback(std::function<void()> _cb)
{
    readCallback = _cb;
}

void Channel::setUseThreadPool(bool use)
{
    useThreadPool = use;
}
