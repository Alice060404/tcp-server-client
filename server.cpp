#include <iostream>

#include "Connection.hpp"
#include "EventLoop.hpp"
#include "Server.hpp"
#include "Socket.hpp"

int main()
{
    EventLoop *loop = new EventLoop();
    Server *server = new Server(loop);
    server->onConnect([](Connection *conn) {
        conn->read();
        if (conn->getState() == Connection::State::Closed)
        {
            conn->close();
            return;
        }
        std::cout << "Message from client " << conn->getSocket()->getFd() << ": " << conn->readBuffer() << '\n';
        conn->setSendBuffer(conn->readBuffer());
        conn->write();
    });
    loop->Loop();

    delete server;
    delete loop;
    return 0;
}
