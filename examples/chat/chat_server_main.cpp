/*
 @ 0xCCCCCCCC
*/

#include "kbase/at_exit_manager.h"
#include "kbase/command_line.h"
#include "kbase/logging.h"

#include "ezio/io_service_context.h"

#include "chat_server.h"

constexpr const kbase::CommandLine::CharType kSwitchPort[] = CMDLINE_LITERAL("port");

int main(int argc, char* argv[])
{
    kbase::AtExitManager exit_manager;

    kbase::CommandLine::Init(argc, argv);
    auto cmdline = kbase::CommandLine::ForCurrentProcess();

    kbase::LoggingSettings logging_settings;
    logging_settings.logging_destination = kbase::LoggingDestination::LogToSystemDebugLog;
    kbase::ConfigureLoggingSettings(logging_settings);

    std::string port("9876");
    IGNORE_RESULT(cmdline.GetSwitchValueASCII(kSwitchPort, port));

    ezio::IOServiceContext::Init();

    ChatServer server(static_cast<unsigned short>(std::stoul(port)));
    server.Start();

    return 0;
}
