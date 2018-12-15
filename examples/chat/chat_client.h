/*
 @ 0xCCCCCCCC
*/

#ifndef CHAT_CHAT_CLIENT_H_
#define CHAT_CHAT_CLIENT_H_

#include "ezio/event_loop.h"
#include "ezio/socket_address.h"
#include "ezio/tcp_client.h"
#include "ezio/thread.h"

#include "data_codec.h"

class ChatClient {
public:
    explicit ChatClient(const ezio::SocketAddress& sockaddr);

    ~ChatClient() = default;

    void Run();

private:
    void ReadUserInput();

    void OnConnection(const ezio::TCPConnectionPtr& conn);

    void OnMessage(const ezio::TCPConnectionPtr&, const std::string& msg, ezio::TimePoint) const;

private:
    ezio::EventLoop main_loop_;
    ezio::Thread thread_;
    ezio::TCPClient client_;
    ezio::TCPConnectionPtr conn_;
    DataCodec codec_;
};

#endif  // CHAT_CHAT_CLIENT_H_
