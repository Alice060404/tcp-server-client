#include "Acceptor.hpp"

#include "Channel.hpp"
#include "InetAddress.hpp"
#include "NetworkConfig.hpp"
#include "Socket.hpp"

#include <functional>

Acceptor::Acceptor(EventLoop *eventLoop) : loop(eventLoop), sock(nullptr), acceptChannel(nullptr)
{
    sock = new Socket();
    InetAddress *addr = new InetAddress(network_config::IP, network_config::PORT);
    sock->bind(addr);
    sock->listen();
    acceptChannel = new Channel(loop, sock);
    std::function<void()> cb = std::bind(&Acceptor::acceptConnection, this);
    acceptChannel->setReadCallback(cb);
    acceptChannel->enableRead();
    delete addr;
}

Acceptor::~Acceptor()
{
    delete acceptChannel;
    delete sock;
}

void Acceptor::acceptConnection()
{
    InetAddress *clientAddr = new InetAddress();
    Socket *clientSock = new Socket(sock->accept(clientAddr));
    clientSock->setNonBlocking();
    if (newConnectionCallback)
        newConnectionCallback(clientSock);
    delete clientAddr;
}

void Acceptor::setNewConnectionCallback(std::function<void(Socket *)> const &callback)
{
    newConnectionCallback = callback;
}
