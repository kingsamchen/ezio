/*
 @ 0xCCCCCCCC
*/

#ifndef SOCKS4A_TUNNEL_H_
#define SOCKS4A_TUNNEL_H_

#include <memory>

#include "kbase/basic_macros.h"

#include "ezio/event_loop.h"
#include "ezio/socket_address.h"
#include "ezio/tcp_client.h"

class Tunnel : public std::enable_shared_from_this<Tunnel> {
public:
    Tunnel(ezio::EventLoop* loop, const ezio::SocketAddress& addr,
           const ezio::TCPConnectionPtr& client_conn);

    ~Tunnel();

    DISALLOW_COPY(Tunnel);

    DISALLOW_MOVE(Tunnel);

    void Connect();

    void Close();

    void Send(kbase::StringView data);

    const std::string& name() const noexcept
    {
        return name_;
    }

private:
    void Setup();

    void Teardown();

    void OnRemoteConnect(const ezio::TCPConnectionPtr& conn) const;

    void OnRemoteDisconnect(const ezio::TCPConnectionPtr& conn) const;

    void OnRemoteMessage(const ezio::TCPConnectionPtr& conn, ezio::Buffer& buf,
                         ezio::TimePoint ts) const;

private:
    std::string name_;
    ezio::TCPClient client_;
    ezio::TCPConnectionPtr client_conn_;
    bool teardown_;
};

using TunnelPtr = std::shared_ptr<Tunnel>;

#endif  // SOCKS4A_TUNNEL_H_
