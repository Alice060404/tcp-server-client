#pragma once

#include <functional>

class Epoll;
class Channel;
class ThreadPool;

class EventLoop
{
  private:
    Epoll *ep;
    ThreadPool *threadPool;
    bool quit;

  public:
    EventLoop();
    ~EventLoop();

    void Loop();

    void updateChannel(Channel *channel);
    void addThread(std::function<void()> func);
};
