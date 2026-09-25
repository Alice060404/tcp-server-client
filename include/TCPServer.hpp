#pragma once

#include "Macros.hpp"

#include <functional>
#include <memory>
#include <unordered_map>
#include <vector>

class EventLoop;
class Socket;
class Acceptor;
class Connection;
class ThreadPool;

class TCPServer
{
  public:
    TCPServer();
    ~TCPServer();

    DISALLOW_COPY_AND_MOVE(TCPServer)

    void start();
    RC newConnection(int fd);
    RC deleteConnection(int fd);
    void onConnect(std::function<void(Connection *)> fn);
    void onRecv(std::function<void(Connection *)> fn);

  private:
    std::unique_ptr<EventLoop> mainReactor_;
    std::unique_ptr<Acceptor> acceptor_;

    std::unordered_map<int, std::unique_ptr<Connection>> connections_;
    std::vector<std::unique_ptr<EventLoop>> subReactors_;

    std::unique_ptr<ThreadPool> threadPool_;

    std::function<void(Connection *)> onConnectCallback;
    std::function<void(Connection *)> onRecvCallback;
};
