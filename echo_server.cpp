#include "Buffer.hpp"
#include "Connection.hpp"
#include "SignalHandler.hpp"
#include "Socket.hpp"
#include "TCPServer.hpp"

#include <csignal>
#include <cstdlib>
#include <iostream>

int main()
{
    TCPServer *server = new TCPServer();

    Signal::signal(SIGINT, [&] {
        delete server;
        std::cout << "Server exit." << '\n';
        exit(0);
    });

    server->onConnect(
        [](Connection *conn) { std::cout << "New connection fd: " << conn->getSocket()->getFd() << '\n'; });

    server->onRecv([](Connection *conn) {
        std::cout << "Message from client " << conn->getReadBuffer()->c_str() << '\n';
        if (conn->getState() == Connection::State::Connected)
            conn->send(conn->getReadBuffer()->c_str());
    });

    server->start();

    delete server;
    return 0;
}
