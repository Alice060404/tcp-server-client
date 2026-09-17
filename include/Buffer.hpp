#pragma once

#include <string>
#include <sys/types.h>

class Buffer
{
  private:
    std::string buf;

  public:
    Buffer();
    ~Buffer();

    void append(const char *_str, int _size);

    ssize_t size() const;
    const char *c_str() const;

    void clear();
    void getline();
};
