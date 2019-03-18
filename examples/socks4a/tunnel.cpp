/*
 @ 0xCCCCCCCC
*/

#include "tunnel.h"

#include <cstring>
#include <functional>

#include "kbase/error_exception_util.h"
#include "kbase/logging.h"

using namespace std::placeholders;

namespace {

void DummyConnectionHandler(const ezio::TCPConnectionPtr&)
{}

void DummyMessageHandler(const ezio::TCPConnectionPtr&, ezio::Buffer&, ezio::TimePoint)
{}

}   // namespace

Tunnel::Tunnel(ezio::EventLoop* loop, const ezio::SocketAddress& addr,
               const ezio::TCPConnectionPtr& client_conn)
    : name_(client_conn->name() + "-tunnel"),
      client_(loop, addr, name_),
      client_conn_(client_conn),
      teardown_(false)
{}

Tunnel::~Tunnel()
{}

void Tunnel::Connect()
{
    Setup();
    client_.Connect();
    LOG(INFO) << "Tunneling " << client_conn_->peer_addr().ToHostPort() << " <-> "
              << client_.remote_addr().ToHostPort();
}

void Tunnel::Close()
{
    LOG(INFO) << "Close tunnel " << client_conn_->peer_addr().ToHostPort() << " <-> "
              << client_.remote_addr().ToHostPort();

    // Still would receive remote events in a short period after calling the Disconnect().
    client_.Disconnect();
    Teardown();
}

void Tunnel::Send(kbase::StringView data)
{
    FORCE_AS_NON_CONST_FUNCTION();
    client_.connection()->Send(data);
}

void Tunnel::Setup()
{
    client_.set_on_connect(std::bind(&Tunnel::OnRemoteConnect, shared_from_this(), _1));
    client_.set_on_disconnect(std::bind(&Tunnel::OnRemoteDisconnect, shared_from_this(), _1));
    client_.set_on_message(std::bind(&Tunnel::OnRemoteMessage, shared_from_this(), _1, _2, _3));
}

void Tunnel::Teardown()
{
    ENSURE(CHECK, !teardown_).Require();
    teardown_ = true;

    // Release binding with the tunnel instance from TCPClient to let the tunnel go for
    // destruction.
    client_.set_on_connect(&DummyConnectionHandler);
    client_.set_on_disconnect(&DummyConnectionHandler);
    client_.set_on_message(&DummyMessageHandler);

    client_conn_ = nullptr;
}

void Tunnel::OnRemoteConnect(const ezio::TCPConnectionPtr& conn) const
{
    DLOG(INFO) << conn->name() << " has connected on thread " << ezio::this_thread::GetName();

    client_conn_->SetTCPNoDelay(true);

    char granted_resp[] = "\x00\x5A\x2A\x2A\x2A\x2A\x2A\x2A";
    auto& addr = client_.remote_addr().raw();
    memcpy(granted_resp + 2, &addr.sin_port, sizeof(addr.sin_port));
    memcpy(granted_resp + 4, &addr.sin_addr.s_addr, sizeof(addr.sin_addr.s_addr));
    client_conn_->Send(kbase::StringView(granted_resp, 8));

    conn->SetTCPNoDelay(true);
}

void Tunnel::OnRemoteDisconnect(const ezio::TCPConnectionPtr& conn) const
{
    DLOG(INFO) << conn->name() << " has disconnected on thread " << ezio::this_thread::GetName();

    if (teardown_) {
        return;
    }

    // Shutdowning the client connection will trigger OnClientDisconnect() on the proxy too.
    ENSURE(CHECK, !!client_conn_).Require();
    client_conn_->Shutdown();
}

void Tunnel::OnRemoteMessage(const ezio::TCPConnectionPtr&, ezio::Buffer& buf,
                             ezio::TimePoint) const
{
    if (teardown_) {
        return;
    }

    ENSURE(CHECK, !!client_conn_).Require();
    client_conn_->Send(buf.ReadAllAsString());
}
