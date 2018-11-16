/*
 @ 0xCCCCCCCC
*/

#include <atomic>
#include <functional>
#include <thread>

#include "kbase/at_exit_manager.h"
#include "kbase/basic_macros.h"
#include "kbase/command_line.h"
#include "kbase/logging.h"

#include "ezio/event_loop.h"
#include "ezio/io_service_context.h"
#include "ezio/tcp_server.h"

using namespace std::placeholders;

class EchoServer {
public:
    EchoServer(unsigned short port, bool multithreaded)
        : tcp_srv_(&io_loop_, ezio::SocketAddress(port), "EchoServer"),
          multithreaded_(multithreaded),
          connections_(0),
          received_messages_(0)
    {
        tcp_srv_.set_on_connection(std::bind(&EchoServer::OnConnection, this, _1));
        tcp_srv_.set_on_message(std::bind(&EchoServer::OnMessage, this, _1, _2, _3));
        io_loop_.RunTaskEvery(std::bind(&EchoServer::DumpServerStatus, this),
                             std::chrono::seconds(5));
    }

    ~EchoServer() = default;

    DISALLOW_COPY(EchoServer);

    DISALLOW_MOVE(EchoServer);

    void Start()
    {
        ezio::TCPServer::Options opt;
        if (multithreaded_) {
            opt.worker_num = std::thread::hardware_concurrency();
        }

        LOG(INFO) << tcp_srv_.name() << " is about to run at " << tcp_srv_.ip_port()
                  << "; multithreading: " << multithreaded_;

        tcp_srv_.Start(opt);

        io_loop_.Run();
    }

private:
    void OnConnection(const ezio::TCPConnectionPtr& conn)
    {
        const char* action = nullptr;
        if (conn->connected()) {
            action = "connected";
            connections_.fetch_add(1);
        } else {
            action = "disconnected";
            connections_.fetch_sub(1);
        }

        LOG(INFO) << conn->name() << " at " << conn->peer_addr().ToHostPort() << " is " << action;

        conn->SetTCPNoDelay(true);
    }

    void OnMessage(const ezio::TCPConnectionPtr& conn, ezio::Buffer& buf, ezio::TimePoint)
    {
        auto msg = buf.ReadAllAsString();
        received_messages_.fetch_add(1);
        conn->Send(msg);
    }

    void DumpServerStatus() const
    {
        LOG(INFO) << "active connections = "
                  << connections_.load(std::memory_order::memory_order_relaxed)
                  << " | received messages = "
                  << received_messages_.load(std::memory_order::memory_order_relaxed);
    }

private:
    ezio::EventLoop io_loop_;
    ezio::TCPServer tcp_srv_;
    bool multithreaded_;
    std::atomic<size_t> connections_;
    std::atomic<size_t> received_messages_;
};

constexpr const kbase::CommandLine::CharType kSwitchPort[]
    = CMDLINE_LITERAL("port");
constexpr const kbase::CommandLine::CharType kSwitchNoMultithread[]
    = CMDLINE_LITERAL("no-multithread");

int main(int argc, char* argv[])
{
    kbase::AtExitManager exit_manager;

    kbase::LoggingSettings logging_settings;
    logging_settings.logging_destination = kbase::LoggingDestination::LogToSystemDebugLog;
    kbase::ConfigureLoggingSettings(logging_settings);

    kbase::CommandLine::Init(argc, argv);

    const auto& cmdline = kbase::CommandLine::ForCurrentProcess();

    std::string port;
    if (!cmdline.GetSwitchValueASCII(kSwitchPort, port)) {
        LOG(ERROR) << "Usage: echo_server --port=<port> [--no-multithread]";
        return 1;
    }

    bool multithreaded = !cmdline.HasSwitch(kSwitchNoMultithread);

    ezio::IOServiceContext::Init();

    EchoServer server(static_cast<unsigned short>(std::stoul(port)), multithreaded);

    server.Start();

    return 0;
}
