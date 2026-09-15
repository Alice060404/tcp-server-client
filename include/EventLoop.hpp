#pragma once

class Epoll;
class Channel;

class EventLoop
{
  private:
    Epoll *ep;
    bool quit;

  public:
    EventLoop();
    ~EventLoop();

    void Loop();

    void updateChannel(Channel *channel);
};
