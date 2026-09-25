#pragma once

#include <cstdint>

// Macros to disable copying and moving
#define DISALLOW_COPY(cname)                                                                                           \
    cname(const cname &) = delete;            /* NOLINT */                                                             \
    cname &operator=(const cname &) = delete; /* NOLINT */

#define DISALLOW_MOVE(cname)                                                                                           \
    cname(cname &&) = delete;            /* NOLINT */                                                                  \
    cname &operator=(cname &&) = delete; /* NOLINT */

#define DISALLOW_COPY_AND_MOVE(cname)                                                                                  \
    DISALLOW_COPY(cname);                                                                                              \
    DISALLOW_MOVE(cname);

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
