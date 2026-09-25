#pragma once

#include "Macros.hpp"

#include <functional>

class EventLoop;
class Socket;

class Channel
{
  public:
    Channel(int fd, EventLoop *loop);
    ~Channel();

    DISALLOW_COPY_AND_MOVE(Channel)

    void enableRead();
    void enableWrite();
    void handleEvent() const;

    int getFd() const;
    short getListenEvents() const;
    short getReadyEvents() const;
    bool getExist() const;
    void setExist(bool in = true);

    void enableET();
    void setReadyEvents(short events);
    void setReadCallback(std::function<void()> &&callback);
    void setReadCallback(std::function<void()> const &callback);
    void setWriteCallback(std::function<void()> &&callback);
    void setWriteCallback(std::function<void()> const &callback);

    static const short READ_EVENT;
    static const short WRITE_EVENT;
    static const short ET;

  private:
    int fd_;
    EventLoop *loop_;
    short listenEvents_;
    short readyEvents_;
    bool exist_;
    std::function<void()> readCallback_;
    std::function<void()> writeCallback_;
};
