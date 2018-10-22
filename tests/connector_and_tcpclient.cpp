/*
 @ 0xCCCCCCCC
*/

#include "catch2/catch.hpp"

#include "ezio/connector.h"

#include <memory>

#include "kbase/at_exit_manager.h"
#include "kbase/error_exception_util.h"

#include "ezio/event_loop.h"
#include "ezio/io_service_context.h"
#include "ezio/socket_address.h"

namespace ezio {

TEST_CASE("Connecting to a remote host", "[Connector]")
{
    kbase::AtExitManager exit_manager;
    IOServiceContext::Init();

    EventLoop main_loop;

    SocketAddress remote_addr("10.23.148.213", 9876);

    std::unique_ptr<Connector> connector(MakeConnector(&main_loop, remote_addr));
    connector->set_on_new_connection([&main_loop](ScopedSocket&& sock, const SocketAddress& addr) {
        printf("connected; local address %s\n",addr.ToHostPort().c_str());
        sock = nullptr;
        main_loop.Quit();
    });

    connector->Connect();

    main_loop.Run();
}

TEST_CASE("Cancel a connecting request", "[Connector")
{
    kbase::AtExitManager exit_manager;
    IOServiceContext::Init();

    EventLoop main_loop;

    SocketAddress remote_addr("192.168.126.134", 9876);

    std::unique_ptr<Connector> connector(MakeConnector(&main_loop, remote_addr));
    connector->set_on_new_connection([](ScopedSocket&& sock, const SocketAddress& addr) {
        ENSURE(CHECK, kbase::NotReached()).Require();
    });

    connector->Connect();

    main_loop.RunTaskAfter([&connector, &main_loop] {
        connector->Cancel();
        main_loop.Quit();
    }, std::chrono::seconds(3));

    main_loop.Run();
}

}   // namespace ezio
