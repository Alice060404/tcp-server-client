#include <arpa/inet.h>
#include <cstdint>
#include <cstring>
#include <netinet/in.h>
#include <strings.h>
#include <sys/socket.h>

#include "InetAddress.hpp"

InetAddress::InetAddress() = default;

InetAddress::InetAddress(const char *ip, uint16_t port)
{
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(ip);
    addr.sin_port = htons(port);
}

sockaddr_in InetAddress::getAddr() const
{
    return addr;
}

const char *InetAddress::getIP() const
{
    return inet_ntoa(addr.sin_addr);
}

uint16_t InetAddress::getPort() const
{
    return ntohs(addr.sin_port);
}

void InetAddress::setAddr(sockaddr_in _addr)
{
    addr = _addr;
}
