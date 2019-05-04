/*
 @ 0xCCCCCCCC
*/

#include <functional>
#include <iostream>

#include "kbase/at_exit_manager.h"
#include "kbase/basic_macros.h"
#include "kbase/command_line.h"
#include "kbase/error_exception_util.h"
#include "kbase/logging.h"
#include "kbase/string_encoding_conversions.h"

#include "ezio/event_loop.h"
#include "ezio/io_service_context.h"
#include "ezio/tcp_client.h"

using namespace std::placeholders;

class EchoClient {
public:
    explicit EchoClient(const ezio::SocketAddress& addr)
        : tcp_client_(&io_loop_, addr, "EchoClient")
    {
        tcp_client_.set_on_connect(std::bind(&EchoClient::OnConnection, this, _1));
        tcp_client_.set_on_disconnect(std::bind(&EchoClient::OnConnection, this, _1));
        tcp_client_.set_on_message(std::bind(&EchoClient::OnReceiveMessage, this, _1, _2, _3));
    }

    ~EchoClient() = default;

    DISALLOW_COPY(EchoClient);

    DISALLOW_MOVE(EchoClient);

    void Run()
    {
        tcp_client_.Connect();
        io_loop_.Run();
    }

private:
    void ReadUserInput()
    {
        ENSURE(CHECK, !!conn_).Require();

        std::string s;
        if (!std::getline(std::cin, s, '\n')) {
            tcp_client_.Disconnect();
            return;
        }

        conn_->Send(s);
    }

    void OnConnection(const ezio::TCPConnectionPtr& conn)
    {
        if (conn->connected()) {
            LOG(INFO) << "Connected to " << conn->peer_addr().ToHostPort() << "; local addr: "
                      << conn->local_addr().ToHostPort();
            conn->SetTCPNoDelay(true);
            conn_ = conn;
            ReadUserInput();
        } else {
            LOG(INFO) << "Disconnect from the server";
            conn_ = nullptr;
            io_loop_.Quit();
        }
    }

    void OnReceiveMessage(const ezio::TCPConnectionPtr&, ezio::Buffer& buf, ezio::TimePoint)
    {
        std::cout << buf.ReadAllAsString() << "\n";
        ReadUserInput();
    }

private:
    ezio::EventLoop io_loop_;
    ezio::TCPClient tcp_client_;
    // TODO: Should we incorporate this connection-ptr into TCPClient?
    ezio::TCPConnectionPtr conn_;
};

struct InvalidCmdArgsHandler {
    std::exception_ptr eptr;

    ~InvalidCmdArgsHandler()
    {
        if (eptr) {
            try {
                std::rethrow_exception(eptr);
            } catch (const std::exception& ex) {
                LOG(ERROR) << "Critical Failure: " << ex.what();
                LOG(INFO) << "Usage: echo-client [ip] [port]";
            }

            exit(1);
        }
    }
};

int main(int argc, char* argv[])
{
    kbase::AtExitManager exit_manager;

    kbase::LoggingSettings logging_settings;
    logging_settings.logging_destination = kbase::LoggingDestination::LogToSystemDebugLog;
    kbase::ConfigureLoggingSettings(logging_settings);

    kbase::AlwaysCheckFirstInDebug(false);

    kbase::CommandLine::Init(argc, argv);

    const auto& cmdline = kbase::CommandLine::ForCurrentProcess();

    std::unique_ptr<ezio::SocketAddress> endpoint;

    // Hnadle tedious command-line args.

    {
        InvalidCmdArgsHandler handler;

        try {
            auto params = cmdline.GetParameters();
            ENSURE(THROW, params.size() >= 2)(params.size()).Require("at least ip and port");

#if defined(OS_WIN)
            auto ip = kbase::WideToASCII(params[0]);
#else
            auto ip = params[0];
#endif
            endpoint = std::make_unique<ezio::SocketAddress>(
                ip, static_cast<unsigned short>(std::stoul(params[1])));
        } catch (const std::exception&) {
            handler.eptr = std::current_exception();
        }
    }

    ezio::IOServiceContext::Init();

    EchoClient client(*endpoint);

    client.Run();

    return 0;
}
