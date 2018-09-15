/*
 @ 0xCCCCCCCC
*/

#ifndef EZIO_EVENT_PUMP_IMPL_WIN_H_
#define EZIO_EVENT_PUMP_IMPL_WIN_H_

#include <vector>

#include <Windows.h>

#include "kbase/basic_macros.h"
#include "kbase/scoped_handle.h"

#include "ezio/event_pump.h"

namespace ezio {

class EventLoop;

class EventPump::Impl {
public:
    explicit Impl(EventLoop*);

    ~Impl();

    DISALLOW_COPY(Impl);

    DISALLOW_MOVE(Impl);

    TimePoint Pump(std::chrono::milliseconds timeout, std::vector<IONotification>& notifications);

    void Wakeup();

    void RegisterNotifier(Notifier* notifier) const;

    void UnregisterNotifier(Notifier* notifier) const;

private:
    void AssociateWithNotifier(const Notifier* notifier) const;

    void FillActiveNotifications(size_t count, std::vector<IONotification>& notifications) const;

private:
    kbase::ScopedWinHandle io_port_;
    std::vector<OVERLAPPED_ENTRY> io_events_;
};

}   // namespace ezio

#endif  // EZIO_EVENT_PUMP_IMPL_WIN_H_
