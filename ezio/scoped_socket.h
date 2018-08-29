/*
 @ 0xCCCCCCCC
*/

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef EZIO_SCOPED_SOCKET_H_
#define EZIO_SCOPED_SOCKET_H_

#include "kbase/basic_macros.h"
#include "kbase/scoped_handle.h"

#if defined(OS_WIN)
#include <WinSock2.h>
#endif

namespace ezio {

#if defined(OS_POSIX)

using SocketTraits = kbase::FDTraits;

#else

struct SocketTraits {
    using Handle = SOCKET;

    SocketTraits() = delete;
    ~SocketTraits() = delete;

    static Handle NullHandle() noexcept
    {
        return INVALID_SOCKET;
    }

    static bool IsValid(Handle handle) noexcept
    {
        return handle != INVALID_SOCKET;
    }

    static void Close(Handle handle) noexcept
    {
        closesocket(handle);
    }
};

#endif

using ScopedSocket = kbase::GenericScopedHandle<SocketTraits>;

}   // namespace ezio

#endif  // EZIO_SCOPED_SOCKET_H_
