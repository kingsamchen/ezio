/*
 @ 0xCCCCCCCC
*/

#include "chat_client.h"

#include <iostream>

#include "kbase/error_exception_util.h"

using namespace std::placeholders;

ChatClient::ChatClient(const ezio::SocketAddress& sockaddr)
    : thread_("client-network"),
      client_(thread_.event_loop(), sockaddr, "ChatClient")
{
    client_.set_on_connection(std::bind(&ChatClient::OnConnection, this, _1));
    client_.set_on_message(std::bind(&DataCodec::OnDataReceive, &codec_, _1, _2, _3));

    codec_.set_on_message(std::bind(&ChatClient::OnMessage, this, _1, _2, _3));
}

void ChatClient::Run()
{
    client_.Connect();
    printf("Wait for connecting...");
    main_loop_.Run();
}

void ChatClient::ReadUserInput()
{
    ENSURE(CHECK, main_loop_.BelongsToCurrentThread()).Require();
    ENSURE(CHECK, !!conn_).Require();

    std::string s;
    if (!std::getline(std::cin, s, '\n')) {
        client_.Disconnect();
        return;
    }

    std::string cmd;
    if (DataCodec::MatchCommand(s, cmd)) {
        conn_->Send(DataCodec::NewCommand(cmd));
    } else {
        conn_->Send(DataCodec::NewMessage(s));
    }

    main_loop_.QueueTask(std::bind(&ChatClient::ReadUserInput, this));
}

void ChatClient::OnConnection(const ezio::TCPConnectionPtr& conn)
{
    if (conn->connected()) {
        conn_ = conn;
        printf("connected!\n");
        main_loop_.RunTask(std::bind(&ChatClient::ReadUserInput, this));
    } else {
        conn_ = nullptr;
        printf("Bye!\n");
        // HACK: ensure calling dtor of `client_` after handling disconnection.
        // TODO: Use a more proper way to eliminate this hack.
        main_loop_.RunTaskAfter(std::bind(&ezio::EventLoop::Quit, &main_loop_),
                                std::chrono::seconds(2));
    }
}

void ChatClient::OnMessage(const ezio::TCPConnectionPtr&, const std::string& msg,
                           ezio::TimePoint) const
{
    ENSURE(CHECK, thread_.event_loop()->BelongsToCurrentThread()).Require();
    printf("%s\n", msg.c_str());
}
