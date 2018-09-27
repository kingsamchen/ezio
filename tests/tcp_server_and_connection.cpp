/*
 @ 0xCCCCCCCC
*/

#include "catch2/catch.hpp"

#include "kbase/at_exit_manager.h"

#include "ezio/io_service_context.h"
#include "ezio/event_loop.h"
#include "ezio/tcp_server.h"

namespace {

using namespace ezio;

void OnConnection(const TCPConnectionPtr& conn)
{
    const char* state = conn->connected() ? "connected" : "disconnected";
    printf("Connection %s is %s\n", conn->peer_addr().ToHostPort().c_str(), state);
}

}   // namespace

namespace ezio {

TEST_CASE("Accept and read", "[TCPServer]")
{
    kbase::AtExitManager exit_manager;
    IOServiceContext::Init();

    SocketAddress addr(9876);

    EventLoop loop;

    TCPServer server(&loop, addr, "Discarder");

    server.set_on_connection(&OnConnection);
    server.set_on_message([](const TCPConnectionPtr& conn, Buffer& buf, TimePoint ts) {
        // Assume no message decoding needed
        auto msg = buf.ReadAllAsString();
        if (msg.find("[quit]") != std::string::npos) {
            printf("bye-bye\n");
            EventLoop::current()->Quit();
        }

        printf("msg from %s: %s\n", conn->peer_addr().ToHostPort().c_str(), msg.c_str());
    });

    server.Start();
    printf("%s is running at %s\n", server.name().c_str(), server.ip_port().c_str());

    loop.Run();
}

TEST_CASE("Echo", "[TCPServer]")
{
    kbase::AtExitManager exit_manager;
    IOServiceContext::Init();

    EventLoop loop;

    SocketAddress addr(9876);
    TCPServer server(&loop, addr, "Echo");

    server.set_on_connection(&OnConnection);
    server.set_on_message([](const TCPConnectionPtr& conn, Buffer& buf, TimePoint ts) {
        auto msg = buf.ReadAllAsString();
        if (msg.find("[quit]") != std::string::npos) {
            conn->Shutdown();
        } else if (msg.find("[poweroff]") != std::string::npos) {
            printf("bye-bye\n");
            EventLoop::current()->Quit();
        }

        printf("[%s]: %s\n", conn->name().c_str(), msg.c_str());
        conn->Send(msg);
    });

    server.Start();
    printf("%s is running at %s\n", server.name().c_str(), server.ip_port().c_str());

    loop.Run();
}

}   // namespace ezio
