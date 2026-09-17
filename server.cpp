#include "Server.hpp"
#include "EventLoop.hpp"

int main()
{
    EventLoop *loop = new EventLoop();
    Server *server = new Server(loop);
    loop->Loop();

    delete server;
    delete loop;

    return 0;
}
