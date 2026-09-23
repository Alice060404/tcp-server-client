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
  private:
    EventLoop *mainReactor;
    Acceptor *acceptor;
    std::map<int, Connection *> connections;
    std::vector<EventLoop *> subReaactor;
    ThreadPool *threadPool;
    std::function<void(Connection *)> onConnectCallback;

  public:
    explicit Server(EventLoop *loop);
    ~Server();

    DISALLOW_COPY_AND_MOVE(Server)

    void newConnection(Socket *serverSock);
    void deleteConnection(Socket *sock);
    void onConnect(std::function<void(Connection *)> fn);
};
