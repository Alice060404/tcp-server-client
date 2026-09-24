#include "Connection.hpp"
#include "EventLoop.hpp"
#include "Server.hpp"
#include "SignalHandler.hpp"
#include "Socket.hpp"

#include <csignal>
#include <cstdlib>
#include <iostream>

int main()
{
    EventLoop *loop = new EventLoop();
    Server *server = new Server(loop);

    Signal::signal(SIGINT, [&] {
        delete server;
        delete loop;
        std::cout << "Server exit." << '\n';
        exit(0);
    });

    server->newConnect(
        [](Connection *conn) { std::cout << "New connection fd: " << conn->getSocket()->getFd() << '\n'; });

    server->onMessage([](Connection *conn) {
        std::cout << "Message from client " << conn->readBuffer() << '\n';
        if (conn->getState() == Connection::State::Connected)
            conn->send(conn->readBuffer());
    });

    loop->Loop();
    return 0;
}
