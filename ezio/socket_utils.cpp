/*
 @ 0xCCCCCCCC
*/

#include "ezio/socket_utils.h"

#include "kbase/error_exception_util.h"

namespace ezio {
namespace socket {

void BindOrThrow(const ScopedSocket& sock, const SocketAddress& listening_addr)
{
    const auto& raw_addr = listening_addr.raw();
    int rv = bind(sock.get(), reinterpret_cast<const sockaddr*>(&raw_addr), sizeof(raw_addr));
    ENSURE(THROW, rv == 0)(GetLastErrorCode())(listening_addr.ToHostPort()).Require();
}

void ListenOrThrow(const ScopedSocket& sock)
{
    int rv = listen(sock.get(), SOMAXCONN);
    ENSURE(THROW, rv == 0)(GetLastErrorCode()).Require();
}

}   // namespace socket
}   // namespace ezio
