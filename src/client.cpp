#include <array>
#include <cstddef>
#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "../include/InetAddress.hpp"
#include "../include/common.hpp"
#include "../include/Socket.hpp"

constexpr std::size_t BUFFER_SIZE = 1024;
constexpr std::size_t PORT = 8888;
constexpr const char *IP = "127.0.0.1";

int main()
{

    Socket *clientSock = new Socket();

    InetAddress *serverAddr = new InetAddress(IP, PORT);

    std::cout << "Connecting to server..." << '\n';
    common::exception::throw_if(
        connect(clientSock->getFd(), reinterpret_cast<sockaddr *>(&serverAddr->addr), serverAddr->addrLen) == -1,
        "Failed to connect.");
    std::cout << "Connect to server successfully." << '\n';

    while (true)
    {
        std::array<char, BUFFER_SIZE> buffer{};
        std::cin.getline(buffer.data(), buffer.size());

        const ssize_t writeBytes = write(clientSock->getFd(), buffer.data(), buffer.size());
        if (writeBytes == -1)
        {
            common::exception::throw_if(true, "Sock already disconnect, can not write.");
            break;
        }

        buffer = {};
        const ssize_t readBytes = read(clientSock->getFd(), buffer.data(), buffer.size());
        if (readBytes > 0)
        {
            std::cout << "Msg from server: " << buffer.data() << '\n';
        }
        else if (readBytes == 0)
        {
            std::cout << "Server disconnect.\n";
            break;
        }
        else if (readBytes == -1)
        {
            common::exception::throw_if(true, "Sock read error.");
            delete clientSock;
        }
    }
    delete clientSock;
    return 0;
}
