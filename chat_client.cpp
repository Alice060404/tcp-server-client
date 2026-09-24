#include "Connection.hpp"
#include "NetworkConfig.hpp"
#include "Socket.hpp"

#include <iostream>

int main()
{
    Socket *sock = new Socket();
    sock->connect(network_config::IP, network_config::PORT);

    Connection *conn = new Connection(nullptr, sock);
    while (true)
    {
        conn->read();
        std::cout << "Message from server: " << conn->readBuffer() << '\n';
    }

    delete conn;
    return 0;
}
