#include "Buffer.hpp"
#include "Connection.hpp"
#include "NetworkConfig.hpp"
#include "Socket.hpp"

#include <iostream>
#include <string>

int main()
{
    Socket *sock = new Socket();
    sock->create();
    sock->connect(network_config::IP, network_config::PORT);

    Connection *conn = new Connection(sock->getFd(), nullptr);

    while (true)
    {
        std::string input;
        std::getline(std::cin, input);
        conn->setSendBuffer(input.c_str());
        conn->write();
        if (conn->getState() == Connection::State::Closed)
        {
            conn->close();
            break;
        }
        conn->read();
        std::cout << "Message from server: " << conn->getReadBuffer()->c_str() << '\n';
    }

    delete conn;
    delete sock;
    return 0;
}
