/*
 @ 0xCCCCCCCC
*/

#ifndef EZIO_TCP_SERVER_H_
#define EZIO_TCP_SERVER_H_

#include <atomic>
#include <string>
#include <unordered_map>

#include "kbase/basic_macros.h"

#include "ezio/acceptor.h"
#include "ezio/common_event_handlers.h"
#include "ezio/socket_address.h"
#include "ezio/tcp_connection.h"

namespace ezio {

class EventLoop;

class TCPServer {
public:
    TCPServer(EventLoop* loop, const SocketAddress& addr, std::string name);

    ~TCPServer();

    DISALLOW_COPY(TCPServer);

    DISALLOW_MOVE(TCPServer);

    // This function is thread-safe and it is no harm to call the function multiple times.
    void Start();

    const std::string& name() const noexcept
    {
        return name_;
    }

    std::string ip_port() const
    {
        return listen_addr_.ToHostPort();
    }

    void set_on_connection(ConnectionEventHandler handler)
    {
        on_connection_ = std::move(handler);
    }

    void set_on_message(MessageEventHandler handler)
    {
        on_message_ = std::move(handler);
    }

private:
    void HandleNewConnection(ScopedSocket&& conn_sock, const SocketAddress& conn_addr);

    void RemoveConnection(const TCPConnectionPtr& conn);

private:
    EventLoop* loop_;
    SocketAddress listen_addr_;
    std::string name_;
    std::atomic<bool> started_;
    Acceptor acceptor_;
    int next_conn_id_;

    using ConnectionMap = std::unordered_map<std::string, TCPConnectionPtr>;

    ConnectionMap connections_;

    ConnectionEventHandler on_connection_;
    MessageEventHandler on_message_;
};

}   // namespace ezio

#endif  // EZIO_TCP_SERVER_H_
