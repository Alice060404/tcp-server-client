#include <array>
#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "Buffer.hpp"
#include "InetAddress.hpp"
#include "Socket.hpp"

constexpr std::size_t BUFFER_SIZE = 1024;
constexpr std::size_t PORT = 8888;
constexpr const char *IP = "127.0.0.1";

int main()
{

    Socket *clientSock = new Socket();
    InetAddress *serverAddr = new InetAddress(IP, PORT);
    clientSock->connect(serverAddr);

    int sockFd = clientSock->getFd();

    Buffer *sendBuffer = new Buffer();
    Buffer *readBuffer = new Buffer();

    while (true)
    {
        sendBuffer->getline();
        const ssize_t writeBytes = write(sockFd, sendBuffer->c_str(), sendBuffer->size());
        if (writeBytes == -1)
        {
            std::cout << "Sock already disconnect, can not write.\n";
            break;
        }

        int alreadyRead = 0;
        std::array<char, BUFFER_SIZE> buf{};
        while (true)
        {
            buf = {};
            const ssize_t readBytes = read(sockFd, buf.data(), buf.size());
            if (readBytes > 0)
            {
                readBuffer->append(buf.data(), readBytes);
                alreadyRead += readBytes;
            }
            else if (readBytes == 0)
            {
                std::cout << "Server disconnect.\n";
                exit(EXIT_SUCCESS);
            }
            if (alreadyRead >= sendBuffer->size())
            {
                std::cout << "Msg from server: " << readBuffer->c_str() << '\n';
                break;
            }
        }
        readBuffer->clear();
    }
    delete clientSock;
    delete serverAddr;

    return 0;
}
