#pragma once

#include <string>
#include <sys/types.h>

#include "Macros.hpp"

class Buffer
{
  private:
    std::string buf;

  public:
    Buffer() = default;
    ~Buffer() = default;

    DISALLOW_COPY_AND_MOVE(Buffer)

    void append(const char *_str, int _size);

    ssize_t size() const;
    const char *c_str() const;

    void clear();
    void getline();
    void setBuf(const char *_buf);
};
