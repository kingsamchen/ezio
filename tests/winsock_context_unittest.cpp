/*
 @ 0xCCCCCCCC
*/

#include "catch2/catch.hpp"

#include "ezio/winsock_context.h"

namespace ezio {

TEST_CASE("General usages", "[WinsockContext]")
{
    WinsockContext winsock_ctx;
    REQUIRE(winsock_ctx.AcceptEx != nullptr);
}

}   // namespace ezio
