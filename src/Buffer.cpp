#include "Buffer.hpp"

#include <iostream>
#include <string>
#include <sys/types.h>

void Buffer::append(const char *data, int length)
{
    for (int i = 0; i < length; ++i)
    {
        if (data[i] == '\0')
            break;
        buf.push_back(data[i]);
    }
}

ssize_t Buffer::size() const
{
    return buf.size();
}

const char *Buffer::c_str() const
{
    return buf.c_str();
}

void Buffer::clear()
{
    buf.clear();
}

void Buffer::getline()
{
    buf.clear();
    std::getline(std::cin, buf);
}

void Buffer::setBuf(const char *data)
{
    buf.clear();
    buf.append(data);
}
