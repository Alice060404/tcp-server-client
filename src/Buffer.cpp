#include <iostream>
#include <string>
#include <sys/types.h>

#include "Buffer.hpp"

Buffer::Buffer()
{
}

Buffer::~Buffer()
{
}

void Buffer::append(const char *_str, int _size)
{
    for (int i = 0; i < _size; ++i)
    {
        buf.push_back(_str[i]);
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
