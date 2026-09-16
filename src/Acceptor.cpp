#include "Channel.hpp"
#include "EventLoop.hpp"
#include "InetAddress.hpp"
#include "Socket.hpp"
#include <cstddef>
#include <functional>

#include "Acceptor.hpp"
#include "Channel.hpp"
#include "EventLoop.hpp"
#include "InetAddress.hpp"
#include "Socket.hpp"

constexpr std::size_t PORT = 8888;
constexpr const char *IP = "127.0.0.1";

Acceptor::Acceptor(EventLoop *_loop) : loop(_loop)
{
    sock = new Socket();
    addr = new InetAddress(IP, PORT);
    sock->bind(addr);
    sock->listen();
    sock->setnonblocking();
    acceptChannel = new Channel(loop, sock->getFd());
    std::function<void()> cb = std::bind(&Acceptor::acceptConnection, this);
    acceptChannel->setCallback(cb);
    acceptChannel->enableReading();
}

Acceptor::~Acceptor()
{
    delete sock;
    delete addr;
    delete acceptChannel;
}

void Acceptor::acceptConnection()
{
    newConnectionCallback(sock);
}

void Acceptor::setNewConnectionCallback(std::function<void(Socket *)> _cb)
{
    newConnectionCallback = _cb;
}
