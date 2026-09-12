#include <arpa/inet.h>
#include <array>
#include <cstddef>
#include <cstring>
#include <netinet/in.h>
#include <strings.h>
#include <sys/socket.h>
#include <unistd.h>

#include <iostream>
#include <sys/types.h>

#include "common.cpp"

constexpr std::size_t BUFFER_SIZE = 1024;

int main()
{
    int sockFd = socket(AF_INET, SOCK_STREAM, 0);
    common::exception::throw_if(sockFd == -1, "Failed to sock.");

    sockaddr_in serverAddr;
    bzero(&serverAddr, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serverAddr.sin_port = htons(8888);

    common::exception::throw_if(bind(sockFd, reinterpret_cast<sockaddr *>(&serverAddr), sizeof(serverAddr)) == -1,
                                "Failed to bind.");

    common::exception::throw_if(listen(sockFd, SOMAXCONN) == -1, "Failed to listen.");

    sockaddr_in clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);
    bzero(&clientAddr, sizeof(clientAddr));

    std::cout << "Waiting for client connect.\n";
    int clientSockFd = accept(sockFd, reinterpret_cast<sockaddr *>(&clientAddr), &clientAddrLen);
    common::exception::throw_if(clientSockFd == -1, "Failed to accept.");

    std::cout << "New client Fd: " << clientSockFd << " IP: " << inet_ntoa(clientAddr.sin_addr)
              << " Port: " << ntohs(clientAddr.sin_port) << ".\n";

    while (true)
    {
        std::array<char, BUFFER_SIZE> buffer{};

        const ssize_t readBytes = read(clientSockFd, buffer.data(), buffer.size());
        if (readBytes > 0)
        {
            std::cout << "Msg from client: " << buffer.data() << '\n';
            write(clientSockFd, buffer.data(), buffer.size());
        }
        else if (readBytes == 0)
        {
            std::cout << "Client disconnect.\n";
            close(clientSockFd);
            break;
        }
        else if (readBytes == -1)
        {
            common::exception::throw_if(true, "Sock read error.");
            close(clientSockFd);
        }
    }

    return 0;
}
