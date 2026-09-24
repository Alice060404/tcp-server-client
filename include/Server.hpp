#pragma once

#include "Macros.hpp"

#include <functional>
#include <map>
#include <vector>

class EventLoop;
class Socket;
class Acceptor;
class Connection;
class ThreadPool;

class Server
{
  public:
    explicit Server(EventLoop *loop);
    ~Server();

    DISALLOW_COPY_AND_MOVE(Server)

    void newConnection(Socket *sock);
    void deleteConnection(Socket *sock);
    void onConnect(std::function<void(Connection *)> fn);
    void onMessage(std::function<void(Connection *)> fn);
    void newConnect(std::function<void(Connection *)> fn);

  private:
    EventLoop *mainReactor;
    Acceptor *acceptor;
    std::map<int, Connection *> connections;
    std::vector<EventLoop *> subReactors;
    ThreadPool *threadPool;
    std::function<void(Connection *)> onConnectCallback;
    std::function<void(Connection *)> onMessageCallback;
    std::function<void(Connection *)> newConnectCallback;
};
