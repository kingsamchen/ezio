/*
 @ 0xCCCCCCCC
*/

#include "catch2/catch.hpp"

#include "ezio/socket_address.h"

namespace ezio {

TEST_CASE("SocketAddress represents the abstraction of sockaddr_in", "[SocketAddress]")
{
    SECTION("create from port only")
    {
        SocketAddress addr(1234);
        REQUIRE(1234 == addr.port());
        REQUIRE("0.0.0.0:1234" == addr.ToHostPort());
    }

    SECTION("create from ip address and port")
    {
        SocketAddress addr("127.0.0.1", 8888);
        REQUIRE(8888 == addr.port());
        REQUIRE("127.0.0.1:8888" == addr.ToHostPort());
    }
}

}   // namespace ezio
