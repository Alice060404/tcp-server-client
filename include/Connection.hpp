#pragma once

#include "Macros.hpp"

#include <cstdint>
#include <functional>
#include <string>

class EventLoop;
class Socket;
class Channel;
class Buffer;

class Connection
{
  public:
    enum State : std::uint8_t
    {
        Invalid = 1,
        Handshaking,
        Connected,
        Closed,
        Failed
    };

    Connection(EventLoop *eventLoop, Socket *socket);
    ~Connection();

    DISALLOW_COPY_AND_MOVE(Connection)

    State getState() const;

    void read();
    void write();
    void send(const std::string &msg);
    void close();

    void setSendBuffer(const char *str);
    Buffer *getReadBuffer() const;
    const char *readBuffer() const;
    Buffer *getSendBuffer() const;
    const char *sendBuffer() const;
    void getlineSendBuffer();
    Socket *getSocket() const;
    void onConnect(std::function<void()> fn);
    void onMessage(std::function<void()> fn);
    void setDeleteConnectionCallback(std::function<void(Socket *)> const &callback);
    void setOnConnectCallback(std::function<void(Connection *)> const &callback);
    void setOnMessageCallback(std::function<void(Connection *)> const &callback);
    void business();

  private:
    EventLoop *loop;
    Socket *sock;
    Channel *channel{nullptr};
    State state_{State::Invalid};
    Buffer *sendBuffer_{nullptr};
    Buffer *readBuffer_{nullptr};
    std::function<void(Socket *)> deleteConnectionCallback;
    std::function<void(Connection *)> onConnectCallback;
    std::function<void(Connection *)> onMessageCallback;

    void readNonBlocking();
    void writeNonBlocking();
    void readBlocking();
    void writeBlocking();
};
