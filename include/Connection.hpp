#pragma once

#include <cstdint>
#include <functional>

#include "Macros.hpp"

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

  private:
    EventLoop *loop;
    Socket *sock;
    Channel *channel{nullptr};
    State state_{State::Invalid};
    Buffer *sendBuffer_{nullptr};
    Buffer *readBuffer_{nullptr};
    std::function<void(Socket *)> deleteConnectionCallback;
    std::function<void(Connection *)> onConnectCallback;

    void readNonBlocking();
    void writeNonBlocking();
    void readBlocking();
    void writeBlocking();

  public:
    Connection(EventLoop *_loop, Socket *_sock);
    ~Connection();

    DISALLOW_COPY_AND_MOVE(Connection)

    State getState() const;

    void read();
    void write();
    void close();

    void setSendBuffer(const char *str);
    Buffer *getReadBuffer() const;
    const char *readBuffer() const;
    Buffer *getSendBuffer() const;
    const char *sendBuffer() const;
    void getlineSendBuffer();
    Socket *getSocket() const;
    void onConnect(std::function<void()> fn);
    void setDeleteConnectionCallback(std::function<void(Socket *)> const &_callback);
    void setOnConnectCallback(std::function<void(Connection *)> const &_callback);
};
