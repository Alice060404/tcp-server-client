#pragma once

#include "Macros.hpp"
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
    std::function<void()> readCallback;
    std::function<void()> writeCallback;

  public:
    Channel(EventLoop *_loop, int _fd);
    ~Channel();

    DISALLOW_COPY_AND_MOVE(Channel)

    void enableReading();
    void handleEvent();

    int getFd() const;
    uint32_t getListenEvents() const;
    uint32_t getReadyEvents() const;
    bool getInepoll() const;

    void useET();
    void setInepoll(bool _in = true);
    void setReadyEvents(uint32_t _ev);
    void setReadCallback(std::function<void()> const &_callback);
};
