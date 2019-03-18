/*
 @ 0xCCCCCCCC
*/

#include <cstdio>

#include "kbase/at_exit_manager.h"
#include "kbase/basic_macros.h"
#include "kbase/command_line.h"
#include "kbase/logging.h"

#include "ezio/event_loop.h"
#include "ezio/io_service_context.h"
#include "ezio/socket_address.h"

#include "socks_proxy.h"

#if defined(OS_POSIX)
#include <signal.h>
#elif defined(OS_WIN)
#include <consoleapi.h>
#endif

namespace {

ezio::EventLoop* main_loop = nullptr;

void InstallSigIntHandler()
{
#if defined(OS_POSIX)
    signal(SIGINT, [](int) {
        main_loop->Quit();
    });
#elif defined(OS_WIN)
    SetConsoleCtrlHandler([](DWORD ctrl_type) -> BOOL {
        if (ctrl_type == CTRL_C_EVENT) {
            main_loop->Quit();
            return TRUE;
        }

        return FALSE;
    }, TRUE);
#endif
}

}   // namespace

int main(int argc, char* argv[])
{
    kbase::AtExitManager exit_manager;

    kbase::CommandLine::Init(argc, argv);
    auto args = kbase::CommandLine::ForCurrentProcess().GetArgs();
    if (args.size() < 2 || args[1].empty()) {
        printf("Usage: socks4a port\n");
        return 1;
    }

    auto port = static_cast<unsigned short>(std::stoi(args[1]));

    kbase::LoggingSettings logging_settings;
    logging_settings.logging_destination = kbase::LoggingDestination::LogToSystemDebugLog;
    kbase::ConfigureLoggingSettings(logging_settings);

    InstallSigIntHandler();

    ezio::IOServiceContext::Init();

    ezio::SocketAddress addr(port);

    ezio::EventLoop loop;
    main_loop = &loop;

    SocksProxy proxy(&loop, addr);
    proxy.Start();

    loop.Run();

    DLOG(INFO) << "Bye-bye!";

    return 0;
}
