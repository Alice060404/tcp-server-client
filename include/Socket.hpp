#pragma once

class InetAddress;
class Socket
{
  private:
    int fd;

  public:
    Socket();
    Socket(int _fd);

    ~Socket();

    void bind(InetAddress *addr);
    void listen();
    void setnonblocking();

    int accept(InetAddress *addr);
    void connect(InetAddress *addr);

    int getFd() const;
};
