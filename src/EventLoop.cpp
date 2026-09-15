#include "Channel.hpp"
#include "Epoll.hpp"
#include <vector>

#include "../include/EventLoop.hpp"

EventLoop::EventLoop() : ep(nullptr), quit(false)
{
    ep = new Epoll();
}

EventLoop::~EventLoop()
{
    delete ep;
}

void EventLoop::Loop()
{
    while (!quit)
    {
        std::vector<Channel *> channels = ep->poll();
        for (auto it = channels.begin(); it != channels.end(); ++it)
        {
            (*it)->handleEvent();
        }
    }
}

void EventLoop::updateChannel(Channel *channel)
{
    ep->updateChannel(channel);
}
