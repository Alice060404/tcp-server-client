#include <cerrno>
#include <cstdlib>
#include <string_view>
#include <system_error>

#include "common.hpp"

void common::exception::throw_if(bool condition, std::string_view msg)
{
    if (condition)
    {
        throw std::system_error(errno, std::generic_category(), std::string(msg));
        exit(EXIT_FAILURE);
    }
}
