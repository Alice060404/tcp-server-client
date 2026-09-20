#pragma once

#include <cstdint>
#include <functional>

class Epoll;
class EventLoop;

class Channel
{
  private:
    EventLoop *loop;
    int fd;
    uint32_t events;
    uint32_t revents;
    bool inEpoll;
    bool useThreadPool;
    std::function<void()> readCallback;
    std::function<void()> writeCallback;

  public:
    Channel(EventLoop *_loop, int _fd);
    ~Channel();

    void enableReading();
    void handleEvent();

    int getFd() const;
    uint32_t getEvents() const;
    uint32_t getRevents() const;
    bool getInepoll() const;

    void useET();
    void setInepoll(bool _in = true);
    void setRevents(uint32_t _ev);
    void setReadCallback(std::function<void()> _cb);
    void setUseThreadPool(bool use = true);
};
