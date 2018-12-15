/*
 @ 0xCCCCCCCC
*/

#include "chat_server.h"

#include "kbase/chrono_util.h"
#include "kbase/error_exception_util.h"
#include "kbase/logging.h"
#include "kbase/string_format.h"
#include "kbase/string_util.h"

#include "ezio/socket_address.h"

using namespace std::placeholders;

ChatServer::ChatServer(unsigned short port)
    : srv_(&loop_, ezio::SocketAddress(port), "ChatServer")
{
    srv_.set_on_connection(std::bind(&ChatServer::OnConnection, this, _1));
    srv_.set_on_message(std::bind(&DataCodec::OnDataReceive, &codec_, _1, _2, _3));

    codec_.set_on_command(std::bind(&ChatServer::OnCommand, this, _1, _2, _3));
    codec_.set_on_message(std::bind(&ChatServer::OnMessage, this, _1, _2, _3));
}

void ChatServer::Start()
{
    ezio::TCPServer::Options opt;
    opt.worker_num = std::thread::hardware_concurrency();

    LOG(INFO) << "Chat-Server is about to run at " << srv_.ip_port();
    srv_.Start(opt);

    loop_.Run();
}

void ChatServer::OnConnection(const ezio::TCPConnectionPtr& conn)
{
    if (conn->connected()) {
        LOG(INFO) << "Client at " << conn->peer_addr().ToHostPort() << " is online!";
        OnSystemBroadcast(kbase::StringFormat("{0} joined the chat!", conn->name()));
        OnUserOnline(conn);
    } else {
        OnUserOffline(conn);
        OnSystemBroadcast(kbase::StringFormat("{0} left the chat!", conn->name()));
        LOG(INFO) << "Client at " << conn->peer_addr().ToHostPort() << " is offline!";
    }
}

void ChatServer::OnUserOnline(const ezio::TCPConnectionPtr& conn)
{
    UserData data(conn->name());

    std::lock_guard<std::mutex> lock(session_mtx_);
    sessions_[conn] = std::move(data);
}

void ChatServer::OnUserOffline(const ezio::TCPConnectionPtr& conn)
{
    std::lock_guard<std::mutex> lock(session_mtx_);
    size_t count = sessions_.erase(conn);
    ENSURE(CHECK, count == 1)(count).Require();
}

void ChatServer::OnCommand(const ezio::TCPConnectionPtr& conn, const std::string& cmd,
                           ezio::TimePoint ts)
{
    auto time = kbase::TimePointToLocalTime(ts);
    auto prefix = kbase::StringPrintf("[%02d:%02d:%02d] *| ", time.first.tm_hour, time.first.tm_min,
                                      time.first.tm_sec);

    if (cmd == kCmdList) {
        std::vector<std::string> members;

        {
            std::lock_guard<std::mutex> lock(session_mtx_);
            for (const auto& session : sessions_) {
                members.push_back(session.second.nickname);
            }
        }

        std::string msg(prefix + "Current members in chat-room:\n");
        msg.append(kbase::JoinString(members, "\n"));

        conn->Send(DataCodec::NewMessage(msg));

        return;
    }

    if (cmd == kCmdName) {
        std::string name;

        {
            std::lock_guard<std::mutex> lock(session_mtx_);
            name = sessions_[conn].nickname;
        }

        std::string msg(prefix + "Your current nickname is " + name);
        conn->Send(DataCodec::NewMessage(msg));

        return;
    }

    if (kbase::StartsWith(cmd, kCmdUseName)) {
        std::vector<std::string> components;
        if (kbase::SplitString(cmd, " ", components) != 2) {
            std::string error(prefix + "Incorrect usage of $USE-NAME$!");
            conn->Send(DataCodec::NewMessage(error));
            return;
        }

        {
            std::lock_guard<std::mutex> lock(session_mtx_);
            sessions_[conn].nickname = components[1];
        }

        std::string msg(prefix + "Your nickname has been changed to " + components[1]);
        conn->Send(DataCodec::NewMessage(msg));

        return;
    }

    conn->Send(DataCodec::NewMessage(kbase::StringFormat("Unrecognized command {0}", cmd)));
}

void ChatServer::OnMessage(const ezio::TCPConnectionPtr& conn, const std::string& msg,
                           ezio::TimePoint ts) const
{
    auto t = kbase::TimePointToLocalTime(ts);
    auto time = kbase::StringPrintf("%02d:%02d:%02d", t.first.tm_hour, t.first.tm_min,
                                    t.first.tm_sec);

    // For simplicity, we use grand lock...
    std::lock_guard<std::mutex> lock(session_mtx_);
    auto message = kbase::StringFormat("[{0}] {1}| {2}", time, sessions_.at(conn).nickname, msg);
    for (const auto& session : sessions_) {
        session.first->Send(DataCodec::NewMessage(message));
    }
}

void ChatServer::OnSystemBroadcast(const std::string& broad_message) const
{
    auto time = kbase::TimePointToLocalTime(std::chrono::system_clock::now());
    auto content = kbase::StringPrintf("[%02d:%02d:%02d] *| %s",
                                       time.first.tm_hour,
                                       time.first.tm_min,
                                       time.first.tm_sec,
                                       broad_message.c_str());

    std::lock_guard<std::mutex> lock(session_mtx_);
    for (const auto& session : sessions_) {
        session.first->Send(DataCodec::NewMessage(content));
    }
}
