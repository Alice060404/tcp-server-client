#include <functional>
#include <vector>

#include "Channel.hpp"
#include "Epoll.hpp"
#include "EventLoop.hpp"
#include "ThreadPool.hpp"

EventLoop::EventLoop() : ep(nullptr), threadPool(nullptr), quit(false)
{
    ep = new Epoll();
    threadPool = new ThreadPool();
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

void EventLoop::addThread(std::function<void()> func)
{
    threadPool->add(func);
}
