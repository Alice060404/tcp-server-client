#include "Acceptor.hpp"

#include "Channel.hpp"
#include "Macros.hpp"
#include "NetworkConfig.hpp"
#include "Socket.hpp"

#include <cassert>
#include <fcntl.h>
#include <functional>
#include <memory>

Acceptor::Acceptor(EventLoop *loop)
{
    socket_ = std::make_unique<Socket>();
    assert(socket_->create() == RC_SUCCESS);
    assert(socket_->bind(network_config::IP, network_config::PORT) == RC_SUCCESS);
    assert(socket_->listen() == RC_SUCCESS);

    channel_ = std::make_unique<Channel>(socket_->getFd(), loop);
    std::function<void()> cb = std::bind(&Acceptor::acceptConnection, this);

    channel_->setReadCallback(cb);
    channel_->enableRead();
}

Acceptor::~Acceptor()
{
}

RC Acceptor::acceptConnection() const
{
    int clntFd = -1;
    if (socket_->accept(clntFd) != RC_SUCCESS)
        return RC_ACCEPTOR_ERROR;

    fcntl(clntFd, F_SETFL, fcntl(clntFd, F_GETFL) | O_NONBLOCK);
    if (newConnectionCallback_)
        newConnectionCallback_(clntFd);
    return RC_SUCCESS;
}

void Acceptor::setNewConnectionCallback(std::function<void(int)> const &callback)
{
    newConnectionCallback_ = callback;
}
