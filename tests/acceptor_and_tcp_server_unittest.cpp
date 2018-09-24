/*
 @ 0xCCCCCCCC
*/

#include "catch2/catch.hpp"

#include "kbase/at_exit_manager.h"

#include "ezio/acceptor.h"
#include "ezio/event_loop.h"
#include "ezio/io_service_context.h"

namespace ezio {

TEST_CASE("Acceptor listens on incoming requests", "[Acceptor]")
{
    kbase::AtExitManager exit_manager;
    IOServiceContext::Init();

    SocketAddress addr(9876);

    EventLoop loop;

    Acceptor acceptor(&loop, addr);
    acceptor.set_on_new_connection([&loop](ScopedSocket&& sock, const SocketAddress& peer_addr) {
        printf("new conn is on %s\n", peer_addr.ToHostPort().c_str());
        sock = nullptr;
        loop.Quit();
    });

    REQUIRE_FALSE(acceptor.listening());

    acceptor.Listen();

    REQUIRE(acceptor.listening());
    printf("Listening on %s\n", addr.ToHostPort().c_str());

    loop.Run();

    printf("Main loop exit\n");
}

}   // namespace ezio
