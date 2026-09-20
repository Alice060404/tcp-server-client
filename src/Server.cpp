#include <arpa/inet.h>
#include <functional>
#include <sys/types.h>
#include <unistd.h>

#include "Acceptor.hpp"
#include "Channel.hpp"
#include "Connection.hpp"
#include "Server.hpp"
#include "Socket.hpp"

Server::Server(EventLoop *_loop) : loop(_loop), acceptor(nullptr)
{
    acceptor = new Acceptor(loop);
    std::function<void(Socket *)> cb = std::bind(&Server::newConnection, this, std::placeholders::_1);
    acceptor->setNewConnectionCallback(cb);
}

Server::~Server()
{
    delete acceptor;
}

void Server::newConnection(Socket *sock)
{
    if (sock->getFd() != -1)
    {
        Connection *conn = new Connection(loop, sock);
        std::function<void(int)> cb = std::bind(&Server::deleteConnection, this, std::placeholders::_1);
        conn->setDeleteConnectionCallback(cb);
        connections[sock->getFd()] = conn;
    }
}

void Server::deleteConnection(int sockFd)
{
    if (sockFd != -1)
    {
        auto it = connections.find(sockFd);
        if (it != connections.end())
        {
            Connection *conn = connections[sockFd];
            connections.erase(sockFd);
            delete conn;
        }
    }
}
