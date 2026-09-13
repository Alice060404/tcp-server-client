#include <arpa/inet.h>
#include <array>
#include <cerrno>
#include <cstddef>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

#include "../include/InetAddress.hpp"
#include "../include/channel.hpp"
#include "../include/common.hpp"
#include "../include/epoll.hpp"
#include "../include/socket.hpp"

constexpr std::size_t BUFFER_SIZE = 1024;
constexpr std::size_t PORT = 8888;
constexpr const char *IP = "127.0.0.1";

void handleReadEvent(int sockfd);

int main()
{
    Socket *serverSock = new Socket();
    InetAddress *serverAddr = new InetAddress(IP, PORT);

    serverSock->bind(serverAddr);
    serverSock->listen();

    Epoll *ep = new Epoll();
    serverSock->setnonblocking();
    Channel *servChannel = new Channel(ep, serverSock->getFd());
    servChannel->enableReading();

    while (true)
    {
        std::vector<Channel *> activeChannels = ep->poll();
        int nfds = activeChannels.size();

        for (int i = 0; i < nfds; ++i)
        {
            int channelFd = activeChannels[i]->getFd();
            if (channelFd == serverSock->getFd())
            {
                InetAddress *clientAddr = new InetAddress();
                Socket *clientSock = new Socket(serverSock->accept(clientAddr));

                std::cout << "New client Fd: " << clientSock->getFd() << " IP: " << inet_ntoa(clientAddr->addr.sin_addr)
                          << " Port: " << ntohs(clientAddr->addr.sin_port) << ".\n";

                clientSock->setnonblocking();
                Channel *clntChannel = new Channel(ep, clientSock->getFd());
                clntChannel->enableReading();
            }
            else if (activeChannels[i]->getRevents() & EPOLLIN)
            {
                handleReadEvent(activeChannels[i]->getFd());
            }
            else
            {
                std::cout << "Something else happened.\n";
            }
        }
    }

    delete serverSock;
    delete serverAddr;
    return 0;
}

void handleReadEvent(int sockfd)
{
    std::array<char, BUFFER_SIZE> buffer{};
    while (true)
    {
        buffer = {};

        const ssize_t readBytes = read(sockfd, buffer.data(), buffer.size());
        if (readBytes > 0)
        {
            std::cout << "Msg from client fd " << sockfd << ": " << buffer.data() << '\n';
            write(sockfd, buffer.data(), buffer.size());
        }
        else if (readBytes == 0)
        {
            std::cout << "Client fd " << sockfd << " disconnect.\n";
            close(sockfd);
            break;
        }
        else if (readBytes == -1 && errno == EINTR)
        {
            std::cout << "Continue read.\n";
            continue;
        }
        else if (readBytes == -1 && ((errno == EAGAIN) || (errno == EWOULDBLOCK)))
        {
            std::cout << "Finish reading once, errno: " << errno << '\n';
            break;
        }
    }
}
