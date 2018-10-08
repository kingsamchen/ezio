/*
 @ 0xCCCCCCCC
*/

#include "ezio/socket_utils.h"

#include <netinet/tcp.h>

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
    if (setsockopt(sock.get(), SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0) {
        auto err = errno;
        LOG(ERROR) << "Set socket SO_REUSEADDR " << enable << " failed: " << err;
        ENSURE(CHECK, kbase::NotReached())(err)(enable).Require();
    }
}

void EnableTCPQuickACK(const ScopedSocket& sock)
{
    int optval = 1;
    if (setsockopt(sock.get(), IPPROTO_TCP, TCP_QUICKACK, &optval, sizeof(optval)) < 0) {
        auto err = errno;
        LOG(ERROR) << "Enable socket TCP_QUICKACK failed: " << err;
        ENSURE(CHECK, kbase::NotReached())(err).Require();
    }
}

void ShutdownWrite(const ScopedSocket& sock)
{
    if (shutdown(sock.get(), SHUT_WR) < 0) {
        LOG(ERROR) << "Failed to shutdown write-side: " << errno;
    }
}

}   // namespace socket
}   // namespace socket
