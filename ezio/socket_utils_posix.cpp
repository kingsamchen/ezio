/*
 @ 0xCCCCCCCC
*/

#include "ezio/socket_utils.h"

#include "kbase/error_exception_util.h"
#include "kbase/logging.h"

namespace ezio {
namespace socket {

ScopedSocket CreateNonBlockingSocket()
{
    ScopedSocket sock_fd(::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0));
    ENSURE(THROW, sock_fd.get() > 0)(errno).Require();
    return sock_fd;
}

void SetReuseAddr(const ScopedSocket& sock, bool enable)
{
    int optval = enable ? 1 : 0;
    int rv = setsockopt(sock.get(), SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
    if (rv < 0) {
        auto err = errno;
        LOG(ERROR) << "Set socket SO_REUSEADDR " << enable << " failed: " << err;
        ENSURE(CHECK, kbase::NotReached())(err)(enable).Require();
    }
}

}   // namespace socket
}   // namespace socket
