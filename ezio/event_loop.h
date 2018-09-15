/*
 @ 0xCCCCCCCC
*/

#ifndef EZIO_EVENT_LOOP_H_
#define EZIO_EVENT_LOOP_H_

#include <atomic>

#include "kbase/basic_macros.h"

#include "ezio/event_pump.h"
#include "ezio/this_thread.h"

namespace ezio {

class Notifier;

class EventLoop {
public:
    EventLoop();

    ~EventLoop();

    DISALLOW_COPY(EventLoop);

    DISALLOW_MOVE(EventLoop);

    void Run();

    void Quit();

    // Get the pointer to the EventLoop for current thread.
    static EventLoop* current() noexcept;

    // Returns true if the EventLoop is owned by current thread.
    // Returns false, otherwise.
    bool BelongsToCurrentThread() const noexcept
    {
        return owner_thread_id_ == this_thread::GetThreadID();
    }

    // It is allowed to register a notifier more than once in order to update its properties
    void RegisterNotifier(Notifier* notifier);

    void UnregisterNotifier(Notifier* notifier);

private:
    std::atomic<bool> is_running_;
    this_thread::ThreadID owner_thread_id_;
    EventPump event_pump_;
};

}   // namespace ezio

#endif  // EZIO_EVENT_LOOP_H_
