/*
 @ 0xCCCCCCCC
*/

#ifndef CHAT_CHAT_SERVER_H_
#define CHAT_CHAT_SERVER_H_

#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>

#include "kbase/basic_macros.h"

#include "ezio/event_loop.h"
#include "ezio/tcp_server.h"

#include "data_codec.h"

class ChatServer {
private:
    struct UserData {
        std::string nickname;

        UserData() = default;

        explicit UserData(std::string uname)
            : nickname(std::move(uname))
        {}
    };

public:
    explicit ChatServer(unsigned short port);

    ~ChatServer() = default;

    DISALLOW_COPY(ChatServer);

    DISALLOW_MOVE(ChatServer);

    void Start();

private:
    void OnConnection(const ezio::TCPConnectionPtr& conn);

    void OnUserOnline(const ezio::TCPConnectionPtr& conn);

    void OnUserOffline(const ezio::TCPConnectionPtr& conn);

    void OnCommand(const ezio::TCPConnectionPtr& conn, const std::string& cmd, ezio::TimePoint ts);

    void OnMessage(const ezio::TCPConnectionPtr& conn, const std::string& msg,
                   ezio::TimePoint ts) const;

    void OnSystemBroadcast(const std::string& broad_message) const;

private:
    ezio::EventLoop loop_;
    ezio::TCPServer srv_;

    mutable std::mutex session_mtx_;
    // Use TCPConnectionPtr as session id.
    std::unordered_map<ezio::TCPConnectionPtr, UserData> sessions_;

    DataCodec codec_;
};

#endif  // CHAT_CHAT_SERVER_H_
