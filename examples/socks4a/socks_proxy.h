/*
 @ 0xCCCCCCCC
*/

#ifndef SOCKS4A_SOCKS_PROXY_H_
#define SOCKS4A_SOCKS_PROXY_H_

#include <mutex>
#include <unordered_map>

#include "ezio/event_loop.h"
#include "ezio/socket_address.h"
#include "ezio/tcp_connection.h"
#include "ezio/tcp_server.h"

#include "tunnel.h"

class SocksProxy {
public:
    SocksProxy(ezio::EventLoop* main_loop, const ezio::SocketAddress& addr);

    ~SocksProxy() = default;

    DISALLOW_COPY(SocksProxy);

    DISALLOW_MOVE(SocksProxy);

    void Start();

private:
    void OnClientConnect(const ezio::TCPConnectionPtr& conn) const;

    void OnClientDisconnect(const ezio::TCPConnectionPtr& conn);

    void OnClientMessage(const ezio::TCPConnectionPtr& conn, ezio::Buffer& buf, ezio::TimePoint ts);

private:
    ezio::EventLoop* main_loop_;
    std::mutex tunnels_mtx_;
    std::unordered_map<std::string, TunnelPtr> tunnels_;
    ezio::TCPServer server_;
};

#endif  // SOCKS4A_SOCKS_PROXY_H_
