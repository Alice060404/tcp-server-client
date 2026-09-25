#include "Buffer.hpp"

#include <string>
#include <sys/types.h>

const std::string &Buffer::buf() const
{
    return buf_;
}

void Buffer::append(const char *str, int size)
{
    for (int i = 0; i < size; ++i)
    {
        if (str[i] == '\0')
            break;
        buf_.push_back(str[i]);
    }
}

size_t Buffer::size() const
{
    return buf_.size();
}

const char *Buffer::c_str() const
{
    return buf_.c_str();
}

void Buffer::clear()
{
    buf_.clear();
}

void Buffer::setBuf(const char *buf)
{
    std::string newBuf(buf);
    buf_.swap(newBuf);
}
