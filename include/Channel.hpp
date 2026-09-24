#pragma once

#include "Macros.hpp"

#include <functional>

class EventLoop;
class Socket;

class Channel
{
  public:
    Channel(EventLoop *eventLoop, Socket *socket);
    ~Channel();

    DISALLOW_COPY_AND_MOVE(Channel)

    void enableRead();
    void enableWrite();
    void handleEvent();

    Socket *getSocket() const;
    int getListenEvents() const;
    int getReadyEvents() const;
    bool getExist() const;
    void setExist(bool in = true);

    void useET();
    void setReadyEvents(int events);
    void setReadCallback(std::function<void()> const &callback);
    void setWriteCallback(std::function<void()> const &callback);

    static const int READ_EVENT;
    static const int WRITE_EVENT;
    static const int ET;

  private:
    EventLoop *loop;
    Socket *socket;
    int listenEvents{0};
    int readyEvents{0};
    bool exist{false};
    std::function<void()> readCallback;
    std::function<void()> writeCallback;
};
