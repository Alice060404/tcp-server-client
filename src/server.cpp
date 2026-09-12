#include <arpa/inet.h>
#include <array>
#include <cerrno>
#include <cstddef>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <netinet/in.h>
#include <strings.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "common.cpp"

constexpr std::size_t BUFFER_SIZE = 1024;
constexpr std::size_t MAX_EVENTS = 1024;
constexpr std::size_t PORT = 8888;
constexpr const char *IP = "127.0.0.1";

void setnonblocking(int fd)
{
    fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);
}

int main()
{
    int sockFd = socket(AF_INET, SOCK_STREAM, 0);
    common::exception::throw_if(sockFd == -1, "Failed to sock.");

    sockaddr_in serverAddr;
    bzero(&serverAddr, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr(IP);
    serverAddr.sin_port = htons(PORT);

    common::exception::throw_if(bind(sockFd, reinterpret_cast<sockaddr *>(&serverAddr), sizeof(serverAddr)) == -1,
                                "Failed to bind.");

    common::exception::throw_if(listen(sockFd, SOMAXCONN) == -1, "Failed to listen.");

    int epollFd = epoll_create1(0);
    common::exception::throw_if(epollFd == -1, "Epoll create error.");

    epoll_event events[MAX_EVENTS], ev;
    bzero(&events, sizeof(events));

    bzero(&ev, sizeof(ev));
    ev.data.fd = sockFd;
    ev.events = EPOLLIN | EPOLLET;
    setnonblocking(sockFd);
    epoll_ctl(epollFd, EPOLL_CTL_ADD, sockFd, &ev);

    while (true)
    {
        int nfds = epoll_wait(epollFd, events, MAX_EVENTS, -1);
        common::exception::throw_if(nfds == -1, "Epoll wait error.");

        for (int i = 0; i < nfds; ++i)
        {
            if (events[i].data.fd == sockFd)
            {
                sockaddr_in clientAddr;
                socklen_t clientAddrLen = sizeof(clientAddr);
                bzero(&clientAddr, sizeof(clientAddr));

                int clientSockFd = accept(sockFd, reinterpret_cast<sockaddr *>(&clientAddr), &clientAddrLen);
                common::exception::throw_if(clientSockFd == -1, "Sock accept error.");
                std::cout << "New client Fd: " << clientSockFd << " IP: " << inet_ntoa(clientAddr.sin_addr)
                          << " Port: " << ntohs(clientAddr.sin_port) << ".\n";

                bzero(&ev, sizeof(ev));
                ev.data.fd = clientSockFd;
                ev.events = EPOLLIN | EPOLLET;
                setnonblocking(clientSockFd);
                epoll_ctl(epollFd, EPOLL_CTL_ADD, clientSockFd, &ev);
            }
            else if (events[i].events & EPOLLIN)
            {
                std::array<char, BUFFER_SIZE> buffer{};
                while (true)
                {
                    buffer = {};

                    const ssize_t readBytes = read(events[i].data.fd, buffer.data(), buffer.size());
                    if (readBytes > 0)
                    {
                        std::cout << "Msg from client fd " << events[i].data.fd << ": " << buffer.data() << '\n';
                        write(events[i].data.fd, buffer.data(), buffer.size());
                    }
                    else if (readBytes == 0)
                    {
                        std::cout << "Client fd " << events[i].data.fd << " disconnect.\n";
                        close(events[i].data.fd);
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
            else
            {
                std::cout << "Something else happened.\n";
            }
        }
    }

    close(sockFd);
    return 0;
}
