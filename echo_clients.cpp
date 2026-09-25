#include "Buffer.hpp"
#include "Connection.hpp"
#include "NetworkConfig.hpp"
#include "Socket.hpp"
#include "ThreadPool.hpp"

#include <cstdio>
#include <functional>
#include <iostream>
#include <string>
#include <unistd.h>

void oneClient(int messageCount, int waitSeconds)
{
    Socket *sock = new Socket();
    sock->create();
    sock->connect(network_config::IP, network_config::PORT);

    Connection *conn = new Connection(sock->getFd(), nullptr);
    sleep(waitSeconds);

    int count = 0;
    while (count < messageCount)
    {
        conn->setSendBuffer("I am client.");
        conn->write();
        if (conn->getState() == Connection::State::Closed)
        {
            conn->close();
            break;
        }
        conn->read();
        std::cout << "msg count " << count++ << ": " << conn->getReadBuffer()->c_str() << '\n';
    }
    delete sock;
    delete conn;
}

int main(int argc, char *argv[])
{
    int threads = 100;
    int messageCount = 100;
    int waitSeconds = 0;
    int option = -1;

    const char *optionSpec = "t:m:w:";
    while ((option = getopt(argc, argv, optionSpec)) != -1)
    {
        switch (option)
        {
        case 't':
            threads = std::stoi(optarg);
            break;
        case 'm':
            messageCount = std::stoi(optarg);
            break;
        case 'w':
            waitSeconds = std::stoi(optarg);
            break;
        case '?':
            printf("error optopt: %c\n", optopt);
            printf("error opterr: %d\n", opterr);
            break;
        default:
            break;
        }
    }

    ThreadPool *threadPool = new ThreadPool(threads);
    std::function<void()> func = std::bind(oneClient, messageCount, waitSeconds);
    for (int i = 0; i < threads; ++i)
    {
        threadPool->add(func);
    }
    delete threadPool;
    return 0;
}
