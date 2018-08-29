/*
 @ 0xCCCCCCCC
*/

#include "catch2/catch.hpp"

#include "ezio/scoped_socket.h"

#if defined(OS_POSIX)
#include <sys/types.h>
#include <sys/socket.h>
#else
#include "ezio/winsock_context.h"
#endif

namespace {

ezio::ScopedSocket::Handle CreateSocket()
{
    auto sock = socket(AF_INET, SOCK_STREAM, 0);
    REQUIRE(ezio::SocketTraits::IsValid(sock));
    return sock;
}

}   // namespace

namespace ezio {

TEST_CASE("Null valid and auto-close", "[ScopedSocket]")
{
#if defined(OS_WIN)
    WinsockContext winsock_ctx;
#endif

    ScopedSocket sock;
    REQUIRE(!static_cast<bool>(sock));

    sock.reset(CreateSocket());
    REQUIRE(static_cast<bool>(sock));

    sock = nullptr;
    REQUIRE(!static_cast<bool>(sock));
}

}   // namespace ezio
