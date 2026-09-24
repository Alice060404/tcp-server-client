#include "Server.hpp"

#include "Acceptor.hpp"
#include "Connection.hpp"
#include "EventLoop.hpp"
#include "Exception.hpp"
#include "Socket.hpp"
#include "ThreadPool.hpp"

#include <cstdint>
#include <functional>
#include <thread>
#include <utility>

Server::Server(EventLoop *eventLoop) : mainReactor(eventLoop), acceptor(nullptr), threadPool(nullptr)
{
    if (mainReactor == nullptr)
        throw Exception(ExceptionType::INVALID, "Main reactor can not be nullptr.");
    acceptor = new Acceptor(mainReactor);
    std::function<void(Socket *)> cb = std::bind(&Server::newConnection, this, std::placeholders::_1);
    acceptor->setNewConnectionCallback(cb);

    int size = static_cast<int>(std::thread::hardware_concurrency());
    threadPool = new ThreadPool(size);
    for (int i = 0; i < size; ++i)
    {
        subReactors.push_back(new EventLoop());
    }
    for (int i = 0; i < size; ++i)
    {
        std::function<void()> subLoop = std::bind(&EventLoop::Loop, subReactors[i]);
        threadPool->add(std::move(subLoop));
    }
}

Server::~Server()
{
    for (EventLoop *each : subReactors)
        delete each;
    delete acceptor;
    delete threadPool;
}

void Server::newConnection(Socket *sock)
{
    if (sock->getFd() == -1)
        throw Exception(ExceptionType::INVALID_SOCKET, "New Connection error, invalid client socket.");

    uint64_t loopIndex = sock->getFd() % subReactors.size();
    Connection *conn = new Connection(subReactors[loopIndex], sock);
    std::function<void(Socket *)> cb = std::bind(&Server::deleteConnection, this, std::placeholders::_1);
    conn->setDeleteConnectionCallback(cb);
    conn->setOnMessageCallback(onMessageCallback);
    connections[sock->getFd()] = conn;
    if (newConnectCallback)
        newConnectCallback(conn);
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

void Server::onMessage(std::function<void(Connection *)> fn)
{
    onMessageCallback = std::move(fn);
}

void Server::newConnect(std::function<void(Connection *)> fn)
{
    newConnectCallback = std::move(fn);
}
