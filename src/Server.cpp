#include <arpa/inet.h>
#include <array>
#include <cstddef>
#include <functional>
#include <iostream>
#include <sys/types.h>
#include <unistd.h>

#include "Acceptor.hpp"
#include "Channel.hpp"
#include "InetAddress.hpp"
#include "Server.hpp"
#include "Socket.hpp"

constexpr std::size_t BUFFER_SIZE = 1024;

Server::Server(EventLoop *_loop) : loop(_loop), acceptor(nullptr)
{
    acceptor = new Acceptor(loop);
    std::function<void(Socket *)> cb = std::bind(&Server::newConnection, this, std::placeholders::_1);
    acceptor->setNewConnectionCallback(cb);
}

Server::~Server()
{
    delete acceptor;
}

void Server::handleReadEvent(int sockFd)
{
    std::array<char, BUFFER_SIZE> buffer{};
    while (true)
    {
        buffer = {};

        const ssize_t readBytes = read(sockFd, buffer.data(), buffer.size());
        if (readBytes > 0)
        {
            std::cout << "Msg from client fd " << sockFd << ": " << buffer.data() << '\n';
            write(sockFd, buffer.data(), buffer.size());
        }
        else if (readBytes == 0)
        {
            std::cout << "Client fd " << sockFd << " disconnect.\n";
            close(sockFd);
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

void Server::newConnection(Socket *serverSock)
{
    InetAddress *clientAddr = new InetAddress();
    Socket *clientSock = new Socket(serverSock->accept(clientAddr));
    std::cout << "New client Fd: " << clientSock->getFd() << " IP: " << inet_ntoa(clientAddr->addr.sin_addr)
              << " Port: " << ntohs(clientAddr->addr.sin_port) << ".\n";
    clientSock->setnonblocking();
    Channel *clntChannel = new Channel(loop, clientSock->getFd());
    std::function<void()> cb = std::bind(&Server::handleReadEvent, this, clientSock->getFd());
    clntChannel->setCallback(cb);
    clntChannel->enableReading();
}
