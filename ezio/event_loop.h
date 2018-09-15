/*
 @ 0xCCCCCCCC
*/

#ifndef EZIO_EVENT_LOOP_H_
#define EZIO_EVENT_LOOP_H_

#include <atomic>
#include <functional>
#include <mutex>
#include <vector>

#include "kbase/basic_macros.h"

#include "ezio/event_pump.h"
#include "ezio/this_thread.h"

namespace ezio {

class Notifier;

class EventLoop {
public:
    using Task = std::function<void()>;

    EventLoop();

    ~EventLoop();

    DISALLOW_COPY(EventLoop);

    DISALLOW_MOVE(EventLoop);

    void Run();

    void Quit();

    // Get the pointer to the EventLoop for current thread.
    static EventLoop* current() noexcept;

    // If the function is called on loop thread, the `task` is then executed immediately
    // within the function.
    // Otherwise, the `task` is queued to the loop thread.
    // This function is thread-safe.
    void RunTask(Task task);

    // Queue the task in the loop thread.
    // The task will be executed shortly after the return from pumping events.
    // This function is thread-safe.
    void QueueTask(Task task);

    // Returns true if the EventLoop is owned by current thread.
    // Returns false, otherwise.
    // This function is thread-safe.
    bool BelongsToCurrentThread() const noexcept
    {
        return owner_thread_id_ == this_thread::GetThreadID();
    }

    // It is allowed to register a notifier more than once in order to update its properties
    void RegisterNotifier(Notifier* notifier);

    void UnregisterNotifier(Notifier* notifier);

private:
    void ProcessPendingTasks();

private:
    std::atomic<bool> is_running_;
    this_thread::ThreadID owner_thread_id_;
    EventPump event_pump_;

    bool executing_pending_task_;

    std::mutex task_queue_mutex_;
    std::vector<Task> task_queue_;
};

}   // namespace ezio

#endif  // EZIO_EVENT_LOOP_H_
