#include "TCPServer.hpp"

#include "Acceptor.hpp"
#include "Connection.hpp"
#include "EventLoop.hpp"
#include "Macros.hpp"
#include "ThreadPool.hpp"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <thread>
#include <utility>

TCPServer::TCPServer()
{
    mainReactor_ = std::make_unique<EventLoop>();
    acceptor_ = std::make_unique<Acceptor>(mainReactor_.get());
    std::function<void(int)> cb = std::bind(&TCPServer::newConnection, this, std::placeholders::_1);
    acceptor_->setNewConnectionCallback(cb);

    unsigned int size = std::thread::hardware_concurrency();
    threadPool_ = std::make_unique<ThreadPool>(size);

    for (size_t i = 0; i < size; ++i)
    {
        std::unique_ptr<EventLoop> subReactor = std::make_unique<EventLoop>();
        subReactors_.push_back(std::move(subReactor));
    }
}

TCPServer::~TCPServer()
{
}

void TCPServer::start()
{
    for (size_t i = 0; i < subReactors_.size(); ++i)
    {
        std::function<void()> subLoop = std::bind(&EventLoop::Loop, subReactors_[i].get());
        threadPool_->add(std::move(subLoop));
    }
    mainReactor_->Loop();
}

RC TCPServer::newConnection(int fd)
{
    assert(fd != -1);
    uint64_t loopIndex = fd % subReactors_.size();

    std::unique_ptr<Connection> conn = std::make_unique<Connection>(fd, subReactors_[loopIndex].get());
    std::function<void(int)> cb = std::bind(&TCPServer::deleteConnection, this, std::placeholders::_1);

    conn->setDeleteConnectionCallback(cb);
    conn->setOnRecvCallback(onRecvCallback);
    connections_[fd] = std::move(conn);
    if (onConnectCallback)
        onConnectCallback(connections_[fd].get());

    return RC_SUCCESS;
}

RC TCPServer::deleteConnection(int fd)
{
    auto it = connections_.find(fd);
    assert(it != connections_.end());
    connections_.erase(fd);
    return RC_SUCCESS;
}

void TCPServer::onConnect(std::function<void(Connection *)> fn)
{
    onConnectCallback = std::move(fn);
}

void TCPServer::onRecv(std::function<void(Connection *)> fn)
{
    onRecvCallback = std::move(fn);
}
