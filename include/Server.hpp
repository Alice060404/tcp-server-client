#pragma once

class EventLoop;
class Socket;
class Acceptor;

class Server
{
  private:
    EventLoop *loop;
    Acceptor *acceptor;

  public:
    Server(EventLoop *loop);
    ~Server();

    void handleReadEvent(int sockFd);

    void newConnection(Socket *serverSock);
};
