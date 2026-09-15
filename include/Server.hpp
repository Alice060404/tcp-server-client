#pragma once

class EventLoop;
class Socket;

class Server
{
  private:
    EventLoop *loop;

  public:
    Server(EventLoop *loop);
    ~Server();

    void handleReadEvent(int sockFd);

    void newConnection(Socket *serverSock);
};
