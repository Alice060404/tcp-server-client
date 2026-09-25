#pragma once

#include "Macros.hpp"

#include <cstdint>
#include <functional>
#include <memory>
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

    Connection(int fd, EventLoop *loop);
    ~Connection();

    DISALLOW_COPY_AND_MOVE(Connection)

    State getState() const;

    RC read();
    RC write();
    RC send(const std::string &msg);
    void close();

    void setSendBuffer(const char *str);
    Buffer *getReadBuffer() const;
    Buffer *getSendBuffer() const;
    Socket *getSocket() const;
    void onConnect(std::function<void()> fn);
    void onMessage(std::function<void()> fn);
    void setDeleteConnectionCallback(std::function<void(int)> const &callback);
    void setDeleteConnectionCallback(std::function<void(int)> &&callback);
    void setOnRecvCallback(std::function<void(Connection *)> const &callback);
    void setOnRecvCallback(std::function<void(Connection *)> &&callback);

  private:
    std::unique_ptr<Socket> socket_;
    std::unique_ptr<Channel> channel_;

    State state_;
    std::unique_ptr<Buffer> readBuffer_;
    std::unique_ptr<Buffer> sendBuffer_;

    std::function<void(int)> deleteConnectionCallback;
    std::function<void(Connection *)> onRecvCallback;

    void business();
    RC readNonBlocking();
    RC writeNonBlocking();
    RC readBlocking();
    RC writeBlocking();
};
