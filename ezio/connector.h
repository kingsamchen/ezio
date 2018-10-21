/*
 @ 0xCCCCCCCC
*/

#ifndef EZIO_CONNECTOR_H_
#define EZIO_CONNECTOR_H_

#include <memory>

#include "kbase/basic_macros.h"

#if defined(OS_POSIX)
#elif defined(OS_WIN)
#include "ezio/connector_win.h"
#endif

namespace ezio {

#if defined(OS_POSIX)
#elif defined(OS_WIN)
using Connector = ConnectorWin;
#endif

inline std::unique_ptr<Connector> MakeConnector(EventLoop* loop, const SocketAddress& addr)
{
    return std::make_unique<Connector>(loop, addr);
}

}   // namespace ezio

#endif  // EZIO_CONNECTOR_H_
