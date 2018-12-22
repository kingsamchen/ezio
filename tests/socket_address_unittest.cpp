/*
 @ 0xCCCCCCCC
*/

#include "catch2/catch.hpp"

#include <cstring>

#include "ezio/socket_address.h"

namespace {

bool operator==(const sockaddr_in& lhs, const sockaddr_in& rhs)
{
    return lhs.sin_family == rhs.sin_family &&
           lhs.sin_port == rhs.sin_port &&
           lhs.sin_addr.s_addr == rhs.sin_addr.s_addr;
}

}   // namespace

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

    SECTION("create from raw sockaddr")
    {
        sockaddr_in saddr;
        memset(&saddr, 0, sizeof(saddr));

        saddr.sin_family = AF_INET;
        saddr.sin_port = HostToNetwork(static_cast<unsigned short>(9876));
        saddr.sin_addr.s_addr = HostToNetwork(INADDR_LOOPBACK);

        SocketAddress addr(saddr);
        REQUIRE(9876 == addr.port());
        REQUIRE("127.0.0.1:9876" == addr.ToHostPort());
    }
}

TEST_CASE("Copy and move SocketAddress instances", "[SocketAddress]")
{
    SECTION("Copy from a SocketAddress")
    {
        SocketAddress addr_x(1234);
        SocketAddress addr_y(addr_x);
        REQUIRE(addr_x.ToHostPort() == addr_y.ToHostPort());
        REQUIRE((addr_x.raw() == addr_y.raw()));

        SocketAddress addr_z("127.0.0.1", 9876);
        REQUIRE_FALSE((addr_z.raw() == addr_x.raw()));

        addr_z = addr_y;
        REQUIRE(addr_z.ToHostPort() == addr_x.ToHostPort());
        REQUIRE((addr_z.raw() == addr_x.raw()));
    }

    SECTION("Move from a SocketAddress")
    {
        SocketAddress addr_x(SocketAddress("127.0.0.1", 9876));
        REQUIRE(addr_x.port() == 9876);
        REQUIRE(addr_x.ToHostPort() == "127.0.0.1:9876");

        SocketAddress addr_y(1234);
        addr_y = std::move(addr_x);
        REQUIRE(addr_y.port() == 9876);
        REQUIRE(addr_y.ToHostPort() == "127.0.0.1:9876");
    }
}

}   // namespace ezio
