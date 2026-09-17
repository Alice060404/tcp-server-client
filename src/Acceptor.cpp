#include <arpa/inet.h>
#include <cstddef>
#include <functional>
#include <iostream>

#include "Acceptor.hpp"
#include "Channel.hpp"
#include "EventLoop.hpp"
#include "InetAddress.hpp"
#include "Socket.hpp"

constexpr std::size_t PORT = 8888;
constexpr const char *IP = "127.0.0.1";

Acceptor::Acceptor(EventLoop *_loop) : loop(_loop), sock(nullptr), acceptChannel(nullptr)
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
    delete addr;
}

Acceptor::~Acceptor()
{
    delete sock;
    delete acceptChannel;
}

void Acceptor::acceptConnection()
{
    InetAddress *clientAddr = new InetAddress();
    Socket *clientSock = new Socket(sock->accept(clientAddr));
    std::cout << "new client fd " << clientSock->getFd() << " IP: " << inet_ntoa(clientAddr->getAddr().sin_addr)
              << " Port: " << ntohs(clientAddr->getAddr().sin_port);
    clientSock->setnonblocking();
    newConnectionCallback(clientSock);
    delete clientAddr;
}

void Acceptor::setNewConnectionCallback(std::function<void(Socket *)> _cb)
{
    newConnectionCallback = _cb;
}
