/*
 @ 0xCCCCCCCC
*/

#include "ezio/socket_address.h"

#include <array>
#include <cstring>

#include "kbase/error_exception_util.h"
#include "kbase/string_format.h"

#include "ezio/socket_utils.h"

#if defined(OS_WIN)
#include <WS2tcpip.h>
#endif

namespace {

constexpr const char kAnyAddr[] = "0.0.0.0";

void StringifyHostPort(const char* ip, unsigned short port, std::string& out)
{
    kbase::StringPrintf(out, "%s:%d", ip, port);
}

}   // namespace

namespace ezio {

SocketAddress::SocketAddress(const sockaddr_in& addr)
    : addr_(addr)
{
    std::array<char, 16> ip {};
    auto ip_ptr = inet_ntop(AF_INET, &addr_.sin_addr, ip.data(), ip.size());
    ENSURE(CHECK, ip_ptr != nullptr)(socket::GetLastErrorCode()).Require();

    StringifyHostPort(ip.data(), NetworkToHost(addr_.sin_port), host_port_);
}

SocketAddress::SocketAddress(unsigned short port)
{
    memset(&addr_, 0, sizeof(addr_));

    addr_.sin_family = AF_INET;
    addr_.sin_port = HostToNetwork(port);
    addr_.sin_addr.s_addr = INADDR_ANY;

    StringifyHostPort(kAnyAddr, port, host_port_);
}

SocketAddress::SocketAddress(const std::string& ip, unsigned short port)
{
    memset(&addr_, 0, sizeof(addr_));

    addr_.sin_family = AF_INET;
    addr_.sin_port = HostToNetwork(port);
    int rv = inet_pton(AF_INET, ip.c_str(), &addr_.sin_addr);
    ENSURE(THROW, rv > 0)(socket::GetLastErrorCode())(ip)(port).Require();

    StringifyHostPort(ip.c_str(), port, host_port_);
}

}   // namespace ezio
