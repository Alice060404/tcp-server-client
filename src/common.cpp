#include "common.hpp"

#include <cerrno>
#include <cstdint>
#include <cstdlib>
#include <string>
#include <string_view>
#include <system_error>

void common::exception::throw_if(bool condition, std::string_view msg)
{
    if (condition)
    {
        throw std::system_error(errno, std::generic_category(), std::string(msg));
        exit(EXIT_FAILURE);
    }
}

enum RC : uint8_t
{
    RC_UNDEFINED,
    RC_SUCCESS,
    RC_SOCKET_ERROR,
    RC_POLLER_ERROR,
    RC_CONNECTION_ERROR,
    RC_ACCEPTOR_ERROR,
    RC_UNIMPLEMENTED
};
