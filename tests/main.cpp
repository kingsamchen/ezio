/*
 @ 0xCCCCCCCC
*/

#define CATCH_CONFIG_RUNNER
#include "catch2/catch.hpp"

#include "kbase/command_line.h"

int main(int argc, char* argv[])
{
#if defined(OS_POSIX)
    kbase::CommandLine::Init(argc, argv);
#elif defined(OS_WIN)
    kbase::CommandLine::Init(0, nullptr);
#endif

    Catch::Session session;
    session.applyCommandLine(argc, argv);

    return session.run();
}