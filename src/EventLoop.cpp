#include "EventLoop.hpp"

#include "Channel.hpp"
#include "Poller.hpp"

#include <memory>

EventLoop::EventLoop()
{
    poller_ = std::make_unique<Poller>();
}

EventLoop::~EventLoop()
{
}

void EventLoop::Loop() const
{
    while (true)
    {
        for (Channel *activeChannel : poller_->poll())
        {
            activeChannel->handleEvent();
        }
    }
}

void EventLoop::updateChannel(Channel *channel) const
{
    poller_->updateChannel(channel);
}

void EventLoop::deleteChannel(Channel *channel) const
{
    poller_->deleteChannel(channel);
}
