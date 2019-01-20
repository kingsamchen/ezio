/*
 @ 0xCCCCCCCC
*/

#include "catch2/catch.hpp"

#include "ezio/connector.h"

#include <memory>

#include "kbase/at_exit_manager.h"
#include "kbase/command_line.h"
#include "kbase/error_exception_util.h"

#include "ezio/event_loop.h"
#include "ezio/io_service_context.h"
#include "ezio/socket_address.h"
#include "ezio/tcp_client.h"

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
    connector->set_on_new_connection([](ScopedSocket&&, const SocketAddress&) {
        ENSURE(CHECK, kbase::NotReached()).Require();
    });

    connector->Connect();

    main_loop.RunTaskAfter([&connector, &main_loop] {
        connector->Cancel();
        main_loop.Quit();
    }, std::chrono::seconds(3));

    main_loop.Run();
}

TEST_CASE("Reuse a connector", "[Connector]")
{
    kbase::AtExitManager exit_manager;
    IOServiceContext::Init();

    EventLoop main_loop;

    std::string ip;
    REQUIRE(kbase::CommandLine::ForCurrentProcess().GetSwitchValueASCII(
        CMDLINE_LITERAL("addr"), ip));

    SocketAddress remote_addr(ip, 9876);

    auto connector(MakeConnector(&main_loop, remote_addr));
    size_t used_count = 0;
    connector->set_on_new_connection([&main_loop, &used_count, self = connector.get()](ScopedSocket&& sock, const SocketAddress& addr) {
        ++used_count;
        printf("connected; local address %s\n", addr.ToHostPort().c_str());
        sock = nullptr;
        if (used_count < 3) {
            printf("waiting for re-connect\n");
            main_loop.RunTaskAfter(std::bind(&Connector::Connect, self), std::chrono::seconds(3));
        } else {
            main_loop.Quit();
        }
    });

    connector->Connect();

    main_loop.Run();
}

TEST_CASE("Create a TCPClient instance and do nothing", "[TCPClient]")
{
    kbase::AtExitManager exit_manager;
    IOServiceContext::Init();

    EventLoop main_loop;

    TCPClient client(&main_loop, SocketAddress("127.0.0.1", 9876), "dummy");
}

TEST_CASE("TCPClient connects to a Echo server", "[TCPClient]")
{
    kbase::AtExitManager exit_manager;
    IOServiceContext::Init();

    EventLoop main_loop;

    TCPClient client(&main_loop, SocketAddress("127.0.0.1", 9876), "CannotConnect");

    client.set_on_connect([](const TCPConnectionPtr& conn) {
        printf("Connected to %s; local addr %s\n", conn->peer_addr().ToHostPort().c_str(),
               conn->local_addr().ToHostPort().c_str());
        conn->SetTCPNoDelay(true);
        conn->Send("Hello this is " + conn->name());
    });

    client.set_on_disconnect([](const TCPConnectionPtr& conn) {
        printf("Disconnected from %s\n", conn->peer_addr().ToHostPort().c_str());
        EventLoop::current()->Quit();
    });

    client.set_on_message([](const TCPConnectionPtr& conn, Buffer& buf, TimePoint) {
        if (buf.readable_size() > 0) {
            auto msg = buf.ReadAllAsString() + "\n";
            printf("-> %s", msg.c_str());
            // Disconnect via TCPConnection, that's risky.
            conn->Shutdown();
        }
    });

    client.Connect();

    main_loop.Run();
}

TEST_CASE("Cancel connecting for a TCPClient", "[TCPClient]")
{
    kbase::AtExitManager exit_manager;
    IOServiceContext::Init();

    EventLoop main_loop;

    TCPClient client(&main_loop, SocketAddress("127.0.0.1", 9876), "CannotConnect");

    client.set_on_connect([](const TCPConnectionPtr&) {
        REQUIRE(kbase::NotReached());
    });

    printf("connecting to %s\n", client.remote_addr().ToHostPort().c_str());

    client.Connect();

    main_loop.RunTaskAfter([&client, &main_loop] {
        printf("Cancel connecting\n");
        client.Cancel();
        main_loop.Quit();
    }, std::chrono::seconds(5));

    main_loop.Run();
}

TEST_CASE("Destruct a connecting TCPClient", "[TCPClient]")
{
    kbase::AtExitManager exit_manager;
    IOServiceContext::Init();

    EventLoop main_loop;

    TCPClient client(&main_loop, SocketAddress("127.0.0.1", 9876), "CannotConnect");

    client.set_on_connect([](const TCPConnectionPtr&) {
        REQUIRE(kbase::NotReached());
    });

    client.set_on_connection_destroy([](const TCPConnectionPtr&) {
        REQUIRE(kbase::NotReached());
    });

    printf("connecting to %s\n", client.remote_addr().ToHostPort().c_str());

    client.Connect();

    main_loop.RunTaskAfter([] {
        EventLoop::current()->Quit();
    }, std::chrono::seconds(5));

    main_loop.Run();

    printf("MainLoop is quiting\n");
}

TEST_CASE("Destruct a connected TCPClient", "[TCPClient]")
{
    kbase::AtExitManager exit_manager;
    IOServiceContext::Init();

    EventLoop main_loop;

    TCPClient client(&main_loop, SocketAddress("127.0.0.1", 9876), "Destructor");

    client.set_on_connect([](const TCPConnectionPtr& conn) {
        printf("Connected to %s; local addr %s\n", conn->peer_addr().ToHostPort().c_str(),
               conn->local_addr().ToHostPort().c_str());
        printf("Wait for destruction of TCPClient\n");
        EventLoop::current()->Quit();
    });

    client.set_on_disconnect([](const TCPConnectionPtr& conn) {
        printf("Disconnected from %s\n", conn->peer_addr().ToHostPort().c_str());
    });

    client.set_on_connection_destroy([&main_loop](const TCPConnectionPtr& conn) {
        printf("%s is about to be destroyed\n", conn->peer_addr().ToHostPort().c_str());
        main_loop.Quit();
    });

    client.set_on_message([](const TCPConnectionPtr&, Buffer&, TimePoint) {});

    client.Connect();

    main_loop.Run();
}

}   // namespace ezio
