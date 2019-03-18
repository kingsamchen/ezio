/*
 @ 0xCCCCCCCC
*/

#include "socks_proxy.h"

#include <algorithm>
#include <cstdint>
#include <string>
#include <thread>

#include "kbase/error_exception_util.h"
#include "kbase/logging.h"
#include "kbase/scope_guard.h"
#include "kbase/string_format.h"

#if defined(OS_POSIX)
#include <netdb.h>
#elif defined(OS_WIN)
#include <WS2tcpip.h>
#endif

namespace {

constexpr const char kProxyName[] = "KSProxy";

constexpr int kProtocolVer = 4;
constexpr int kCommandConnect = 1;

constexpr const char kRejectedResp[] = "\x00\x5B\x2B\x2B\x2B\x2B\x2B\x2B";

struct RequestPacket {
    int ver;
    int command;
    uint16_t dest_port;
    uint32_t dest_ip;
    std::string domain;

    RequestPacket()
        : ver(0), command(0), dest_port(0), dest_ip(0)
    {}

    bool IsSocks4a() const noexcept
    {
        return !domain.empty();
    }
};

// Returns true, if the hostname is resolved;
// Returns false, otherwise, and `ip` remains untouched.
bool ResolveHostname(const std::string& hostname, uint32_t& ip)
{
    struct addrinfo hints {};
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = 0;
    hints.ai_flags = 0;

    struct addrinfo* result = nullptr;
    int rv = getaddrinfo(hostname.c_str(), nullptr, &hints, &result);
    if (rv != 0) {
        return false;
    }

    ON_SCOPE_EXIT {
        if (result) {
            freeaddrinfo(result);
        }
    };

    ip = ezio::NetworkToHost(
        static_cast<uint32_t>(reinterpret_cast<sockaddr_in*>(result->ai_addr)->sin_addr.s_addr));

    return true;
}

// If didn't decode a packet, `buf` is untouched.
std::pair<bool, RequestPacket> DecodeRequestPacket(ezio::Buffer& buf)
{
    if (buf.readable_size() <= 8) {
        return {false, {}};
    }

    // USERID field.
    auto userid_delim = std::find(buf.cbegin() + 8, buf.cend(), '\0');
    if (userid_delim == buf.cend()) {
        return {false, {}};
    }

    auto socks4a = false;
    auto domain_delim = buf.cend();
    auto ip_in_network = *reinterpret_cast<const uint32_t*>(buf.Peek() + 4);
    if (ezio::NetworkToHost(ip_in_network) < 256) {
        // Socks4a packet must contain a DOMAIN field.
        socks4a = true;
        domain_delim = std::find(userid_delim + 1, buf.cend(), '\0');
        if (domain_delim == buf.cend()) {
            return {false, {}};
        }
    }

    RequestPacket packet;

    packet.ver = buf.ReadAsInt8();
    packet.command = buf.ReadAsInt8();
    packet.dest_port = static_cast<uint16_t>(buf.ReadAsInt16());
    packet.dest_ip = static_cast<uint32_t>(buf.ReadAsInt32());

    if (socks4a) {
        packet.domain.assign(std::next(userid_delim), domain_delim);
        buf.Consume(static_cast<size_t>(domain_delim - buf.cbegin() + 1));
    } else {
        buf.Consume(static_cast<size_t>(userid_delim - buf.cbegin() + 1));
    }

    return {true, packet};
}

}   // namesapce

using namespace std::placeholders;

SocksProxy::SocksProxy(ezio::EventLoop* main_loop, const ezio::SocketAddress& addr)
    : main_loop_(main_loop),
      server_(main_loop_, addr, kProxyName)
{
    server_.set_on_connect(std::bind(&SocksProxy::OnClientConnect, this, _1));
    server_.set_on_disconnect(std::bind(&SocksProxy::OnClientDisconnect, this, _1));
    server_.set_on_message(std::bind(&SocksProxy::OnClientMessage, this, _1, _2, _3));
}

void SocksProxy::Start()
{
    ezio::TCPServer::Options opt;
    opt.worker_num = std::thread::hardware_concurrency();

    server_.Start(opt);

    LOG(INFO) << server_.name() << " is listening at " << server_.ip_port();
}

void SocksProxy::OnClientConnect(const ezio::TCPConnectionPtr& conn) const
{
    FORCE_AS_MEMBER_FUNCTION();
    DLOG(INFO) << conn->name() << " has connected on thread " << ezio::this_thread::GetName();
}

void SocksProxy::OnClientDisconnect(const ezio::TCPConnectionPtr& conn)
{
    DLOG(INFO) << conn->name() << " has disconnected on thread " << ezio::this_thread::GetName();

    TunnelPtr tunnel;

    {
        std::lock_guard<std::mutex> lock(tunnels_mtx_);
        auto it = tunnels_.find(conn->name());
        if (it != tunnels_.end()) {
            tunnel = std::move(it->second);
            tunnels_.erase(it);
            DLOG(INFO) << tunnel->name() << " has been removed";
        }
    }

    if (!tunnel) {
        LOG(WARNING) << conn->name() << " didn't have a corresponding tunnel";
        return;
    }

    tunnel->Close();
}

void SocksProxy::OnClientMessage(const ezio::TCPConnectionPtr& conn, ezio::Buffer& buf,
                                 ezio::TimePoint)
{
    TunnelPtr tunnel;

    {
        std::lock_guard<std::mutex> lock(tunnels_mtx_);
        auto it = tunnels_.find(conn->name());
        if (it != tunnels_.end()) {
            tunnel = it->second;
        }
    }

    if (tunnel) {
        tunnel->Send(buf.ReadAllAsString());
        return;
    }

    // The client is new.

    auto result = DecodeRequestPacket(buf);
    if (!result.first) {
        return;
    }

    auto& request = result.second;
    if (request.ver != kProtocolVer ||
        request.command != kCommandConnect ||
        (request.IsSocks4a() && !ResolveHostname(request.domain, request.dest_ip))) {
        conn->Send(kbase::StringView(kRejectedResp, 8));
        conn->Shutdown();
        return;
    }

    // The request is verified, now create a tunnel.

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = ezio::HostToNetwork(request.dest_port);
    addr.sin_addr.s_addr = ezio::HostToNetwork(request.dest_ip);

    tunnel = std::make_shared<Tunnel>(
        ezio::EventLoop::current(), ezio::SocketAddress(addr), conn);
    tunnel->Connect();

    {
        std::lock_guard<std::mutex> lock(tunnels_mtx_);
        tunnels_[conn->name()] = tunnel;
    }
}
