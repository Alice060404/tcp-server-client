#include "EventLoop.hpp"

#include "Channel.hpp"
#include "Poller.hpp"

#include <vector>

EventLoop::EventLoop()
{
    poller_ = new Poller();
}

EventLoop::~EventLoop()
{
    quit();
    delete poller_;
}

void EventLoop::Loop()
{
    while (!quit_)
    {
        std::vector<Channel *> channels = poller_->poll();
        for (auto &ch : channels)
        {
            ch->handleEvent();
        }
    }
}

void EventLoop::quit()
{
    quit_ = true;
}

void EventLoop::updateChannel(Channel *channel)
{
    poller_->updateChannel(channel);
}

void EventLoop::deleteChannel(Channel *channel)
{
    poller_->deleteChannel(channel);
}
