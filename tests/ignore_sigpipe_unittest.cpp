/*
 @ 0xCCCCCCCC
*/

#include "catch2/catch.hpp"

#include "ezio/ignore_sigpipe.h"

#include <unistd.h>

#include "kbase/scoped_handle.h"

namespace ezio {

TEST_CASE("Ignore sigpipe handler", "[IgnoreSigPipe]")
{
    int fds[2] {0};
    int rv = pipe(fds);

    CHECK(rv == 0);
    INFO("Failed to call pipe(): " << errno);

    IgnoreSigPipe ignore_sigpipe;
    UNUSED_VAR(ignore_sigpipe);

    kbase::ScopedFD read_end(fds[0]);
    kbase::ScopedFD write_end(fds[1]);

    // Close read end.
    read_end = nullptr;

    int n = 1;
    ssize_t result = write(write_end.get(), &n, sizeof(n));

    REQUIRE(result == -1);
    REQUIRE(errno == EPIPE);
}

}   // namespace ezio
