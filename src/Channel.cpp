#include "Channel.hpp"

#include "EventLoop.hpp"

#include <functional>
#include <sys/epoll.h>
#include <utility>

const short Channel::READ_EVENT = 1;
const short Channel::WRITE_EVENT = 2;
const short Channel::ET = 4;

Channel::Channel(int fd, EventLoop *loop) : fd_(fd), loop_(loop), listenEvents_(0), readyEvents_(0), exist_(false)
{
}

Channel::~Channel()
{
    loop_->deleteChannel(this);
}

void Channel::enableRead()
{
    listenEvents_ |= READ_EVENT;
    loop_->updateChannel(this);
}

void Channel::enableWrite()
{
    listenEvents_ |= WRITE_EVENT;
    loop_->updateChannel(this);
}

void Channel::enableET()
{
    listenEvents_ |= ET;
    loop_->updateChannel(this);
}

int Channel::getFd() const
{
    return fd_;
}

short Channel::getListenEvents() const
{
    return listenEvents_;
}

short Channel::getReadyEvents() const
{
    return readyEvents_;
}

bool Channel::getExist() const
{
    return exist_;
}

void Channel::setExist(bool in)
{
    exist_ = in;
}

void Channel::setReadyEvents(short events)
{
    if (events & READ_EVENT)
        readyEvents_ |= READ_EVENT;
    if (events & WRITE_EVENT)
        readyEvents_ |= WRITE_EVENT;
    if (events & ET)
        readyEvents_ |= ET;
}

void Channel::handleEvent() const
{
    if (readyEvents_ & READ_EVENT)
    {
        readCallback_();
    }
    if (readyEvents_ & WRITE_EVENT)
    {
        writeCallback_();
    }
}

void Channel::setReadCallback(std::function<void()> &&callback)
{
    readCallback_ = std::move(callback);
}

void Channel::setReadCallback(std::function<void()> const &callback)
{
    readCallback_ = callback;
}

void Channel::setWriteCallback(std::function<void()> &&callback)
{
    writeCallback_ = std::move(callback);
}

void Channel::setWriteCallback(std::function<void()> const &callback)
{
    writeCallback_ = callback;
}
