#include <arpa/inet.h>
#include <cstddef>
#include <netinet/in.h>
#include <strings.h>
#include <sys/socket.h>

#include "InetAddress.hpp"

InetAddress::InetAddress() : addrLen(sizeof(addr))
{
    bzero(&addr, sizeof(addr));
}

InetAddress::InetAddress(const char *ip, std::size_t port) : addrLen(sizeof(addr))
{
    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(ip);
    addr.sin_port = htons(port);
}

InetAddress::~InetAddress() {};
