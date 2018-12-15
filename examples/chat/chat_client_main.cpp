/*
 @ 0xCCCCCCCC
*/

#include <iostream>

#include "kbase/at_exit_manager.h"
#include "kbase/command_line.h"
#include "kbase/logging.h"

#include "ezio/io_service_context.h"
#include "ezio/socket_address.h"

#include "chat_client.h"

int main(int argc, char* argv[])
{
    kbase::AtExitManager exit_manager;

    kbase::LoggingSettings logging_settings;
    logging_settings.logging_destination = kbase::LoggingDestination::LogToSystemDebugLog;
    kbase::ConfigureLoggingSettings(logging_settings);

    kbase::CommandLine::Init(argc, argv);

    auto cmdline = kbase::CommandLine::ForCurrentProcess();
    auto args = cmdline.GetParameters();
    if (args.size() < 2) {
        std::cerr << "Usage: chat-client ip port" << std::endl;
        return 1;
    }

    ezio::IOServiceContext::Init();

    ezio::SocketAddress addr(std::string(args[0].begin(), args[0].end()).c_str(),
                             static_cast<unsigned short>(std::stoul(args[1])));

    ChatClient client(addr);

    client.Run();

    return 0;
}
