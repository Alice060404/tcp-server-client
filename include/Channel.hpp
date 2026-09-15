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
    std::function<void()> callback;

  public:
    Channel(EventLoop *_loop, int _fd);
    ~Channel();

    void enableReading();
    void handleEvent();

    int getFd() const;
    uint32_t getEvents() const;
    uint32_t getRevents() const;
    bool getInepoll() const;

    void setInepoll();
    void setRevents(uint32_t _ev);
    void setCallback(std::function<void()> _cb);
};
