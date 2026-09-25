#pragma once

#include "Macros.hpp"

#include <string>
#include <sys/types.h>

class Buffer
{
  public:
    Buffer() = default;
    ~Buffer() = default;

    DISALLOW_COPY_AND_MOVE(Buffer)

    void append(const char *data, int length);

    const std::string &buf() const;
    size_t size() const;
    const char *c_str() const;

    void clear();
    void setBuf(const char *data);

  private:
    std::string buf_;
};
