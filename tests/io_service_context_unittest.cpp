/*
 @ 0xCCCCCCCC
*/

#include "catch2/catch.hpp"

#include "ezio/io_service_context.h"

#include "kbase/at_exit_manager.h"

#if defined(OS_POSIX)
#include <unistd.h>

#include "kbase/scoped_handle.h"
#endif

namespace ezio {

TEST_CASE("Init IOServiceContext before using any facilities", "[IOServiceContext]")
{
    kbase::AtExitManager exit_manager;
    IOServiceContext::Init();

#if defined(OS_WIN)
    SECTION("access winsock-context via io-service-context")
    {
        auto pfn = IOServiceContext::current().AsWinsockContext().AcceptEx;
        REQUIRE(pfn != nullptr);
    }
#else
    SECTION("io-service-context on Linux uses IgnoreSigpipe")
    {
        int fds[2] {0};
        int rv = pipe(fds);

        CHECK(rv == 0);
        INFO("Failed to call pipe(): " << errno);

        kbase::ScopedFD read_end(fds[0]);
        kbase::ScopedFD write_end(fds[1]);

        // Close read end.
        read_end = nullptr;

        int n = 1;
        ssize_t result = write(write_end.get(), &n, sizeof(n));

        REQUIRE(result == -1);
        REQUIRE(errno == EPIPE);
    }
#endif
}

}   // namespace ezio
