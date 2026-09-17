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
    Connection *conn = new Connection(loop, sock);
    std::function<void(Socket *)> cb = std::bind(&Server::deleteConnection, this, std::placeholders::_1);
    conn->setDeleteConnectionCallback(cb);
    connections[sock->getFd()] = conn;
}

void Server::deleteConnection(Socket *sock)
{
    Connection *conn = connections[sock->getFd()];
    connections.erase(sock->getFd());
    delete conn;
}
