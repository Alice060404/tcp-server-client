#include <arpa/inet.h>
#include <cstdint>
#include <functional>
#include <sys/types.h>
#include <thread>
#include <unistd.h>
#include <utility>

#include "Acceptor.hpp"
#include "Channel.hpp"
#include "Connection.hpp"
#include "EventLoop.hpp"
#include "Server.hpp"
#include "Socket.hpp"
#include "ThreadPool.hpp"
#include "common.hpp"

Server::Server(EventLoop *_loop) : mainReactor(_loop), acceptor(nullptr), threadPool(nullptr)
{
    acceptor = new Acceptor(mainReactor);
    std::function<void(Socket *)> cb = std::bind(&Server::newConnection, this, std::placeholders::_1);
    acceptor->setNewConnectionCallback(cb);

    int size = static_cast<int>(std::thread::hardware_concurrency());
    threadPool = new ThreadPool(size);
    for (int i = 0; i < size; ++i)
    {
        subReaactor.push_back(new EventLoop());
    }
    for (int i = 0; i < size; ++i)
    {
        std::function<void()> subLoop = std::bind(&EventLoop::Loop, subReaactor[i]);
        threadPool->add(std::move(subLoop));
    }
}

Server::~Server()
{
    delete acceptor;
    delete threadPool;
}

void Server::newConnection(Socket *sock)
{
    common::exception::throw_if(sock->getFd() == -1, "New connection error.");
    uint64_t random = sock->getFd() % subReaactor.size();
    Connection *conn = new Connection(subReaactor[random], sock);
    std::function<void(Socket *)> cb = std::bind(&Server::deleteConnection, this, std::placeholders::_1);
    conn->setDeleteConnectionCallback(cb);
    conn->setOnConnectCallback(onConnectCallback);
    connections[sock->getFd()] = conn;
}

void Server::deleteConnection(Socket *sock)
{
    int sockFd = sock->getFd();
    auto it = connections.find(sockFd);
    if (it != connections.end())
    {
        Connection *conn = connections[sockFd];
        connections.erase(sockFd);
        delete conn;
        conn = nullptr;
    }
}

void Server::onConnect(std::function<void(Connection *)> fn)
{
    onConnectCallback = std::move(fn);
}
