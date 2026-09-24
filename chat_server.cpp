#include "Connection.hpp"
#include "EventLoop.hpp"
#include "Server.hpp"
#include "Socket.hpp"

#include <iostream>
#include <map>

int main()
{
    std::map<int, Connection *> clients;

    EventLoop *loop = new EventLoop();
    Server *server = new Server(loop);

    server->newConnect([&](Connection *conn) {
        int clntFd = conn->getSocket()->getFd();
        std::cout << "New Connection fd: " << clntFd << '\n';
        clients[clntFd] = conn;
        for (auto &each : clients)
        {
            Connection *client = each.second;
            client->send(conn->readBuffer());
        }
    });

    server->onMessage([&](Connection *conn) {
        std::cout << "Message from client " << conn->readBuffer() << '\n';
        for (auto &each : clients)
        {
            Connection *client = each.second;
            client->send(conn->readBuffer());
        }
    });

    loop->Loop();
    return 0;
}
