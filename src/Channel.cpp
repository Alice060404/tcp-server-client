#include "Channel.hpp"

#include "EventLoop.hpp"
#include "Socket.hpp"

#include <functional>
#include <sys/epoll.h>

const int Channel::READ_EVENT = 1;
const int Channel::WRITE_EVENT = 2;
const int Channel::ET = 4;

Channel::Channel(EventLoop *eventLoop, Socket *socket) : loop(eventLoop), socket(socket)
{
}

Channel::~Channel()
{
    loop->deleteChannel(this);
}

void Channel::enableRead()
{
    listenEvents |= READ_EVENT;
    loop->updateChannel(this);
}

void Channel::enableWrite()
{
    listenEvents |= WRITE_EVENT;
    loop->updateChannel(this);
}

void Channel::useET()
{
    listenEvents |= ET;
    loop->updateChannel(this);
}

Socket *Channel::getSocket() const
{
    return socket;
}

int Channel::getListenEvents() const
{
    return listenEvents;
}

int Channel::getReadyEvents() const
{
    return readyEvents;
}

bool Channel::getExist() const
{
    return exist;
}

void Channel::setExist(bool in)
{
    exist = in;
}

void Channel::setReadyEvents(int events)
{
    if (events & READ_EVENT)
        readyEvents |= READ_EVENT;
    if (events & WRITE_EVENT)
        readyEvents |= WRITE_EVENT;
    if (events & ET)
        readyEvents |= ET;
}

void Channel::handleEvent()
{
    if (readyEvents & (EPOLLIN | EPOLLPRI))
    {
        readCallback();
    }
    if (readyEvents & (EPOLLOUT))
    {
        writeCallback();
    }
}

void Channel::setReadCallback(std::function<void()> const &callback)
{
    readCallback = callback;
}

void Channel::setWriteCallback(std::function<void()> const &callback)
{
    writeCallback = callback;
}
