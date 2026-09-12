#include <arpa/inet.h>
#include <array>
#include <cstddef>
#include <iostream>
#include <netinet/in.h>
#include <strings.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "common.cpp"

constexpr std::size_t BUFFER_SIZE = 1024;
constexpr std::size_t PORT = 8888;
constexpr const char *IP = "127.0.0.1";

int main()
{
    int sockFd = socket(AF_INET, SOCK_STREAM, 0);
    common::exception::throw_if(sockFd == -1, "Failed to sock.");

    sockaddr_in serverAddr;
    bzero(&serverAddr, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr(IP);
    serverAddr.sin_port = htons(PORT);

    std::cout << "Connecting to server..." << '\n';
    common::exception::throw_if(connect(sockFd, reinterpret_cast<sockaddr *>(&serverAddr), sizeof(serverAddr)),
                                "Failed to connect.");
    std::cout << "Connect to server successfully." << '\n';

    while (true)
    {
        std::array<char, BUFFER_SIZE> buffer{};
        std::cin.getline(buffer.data(), buffer.size());

        const ssize_t writeBytes = write(sockFd, buffer.data(), buffer.size());
        if (writeBytes == -1)
        {
            common::exception::throw_if(true, "Sock already disconnect, can not write.");
            break;
        }

        buffer = {};
        const ssize_t readBytes = read(sockFd, buffer.data(), buffer.size());
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
            close(sockFd);
        }
    }
    return 0;
}
