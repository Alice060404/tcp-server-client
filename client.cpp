#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "Connection.hpp"
#include "Socket.hpp"

constexpr std::size_t PORT = 8888;
constexpr const char *IP = "127.0.0.1";

int main()
{
    Socket *clientSock = new Socket();
    clientSock->connect(IP, PORT);

    Connection *conn = new Connection(nullptr, clientSock);

    while (true)
    {
        conn->getlineSendBuffer();
        conn->write();
        if (conn->getState() == Connection::State::Closed)
        {
            conn->close();
            break;
        }
        conn->read();
        std::cout << "Message from server: " << conn->readBuffer() << '\n';
    }
    delete conn;
    return 0;
}
