/*
 @ 0xCCCCCCCC
*/

#include "ezio/event_loop.h"

#include <vector>

#include "kbase/error_exception_util.h"
#include "kbase/scope_guard.h"

#include "ezio/notifier.h"

namespace {

constexpr auto kPumpTime = std::chrono::seconds(10);

}   // namespace

namespace ezio {

thread_local EventLoop* tls_loop_in_thread {nullptr};

EventLoop::EventLoop()
    : is_running_(false),
      owner_thread_id_(this_thread::GetThreadID()),
      event_pump_(this),
      executing_pending_task_(false)
{
    ENSURE(CHECK, tls_loop_in_thread == nullptr).Require();
    tls_loop_in_thread = this;
}

EventLoop::~EventLoop()
{
    ENSURE(CHECK, tls_loop_in_thread != nullptr).Require();
    tls_loop_in_thread = nullptr;
}

void EventLoop::Run()
{
    ENSURE(CHECK, !is_running_.load()).Require();
    ENSURE(CHECK, BelongsToCurrentThread()).Require();

    std::vector<IONotification> active_notifications;

    is_running_.store(true, std::memory_order_release);
    while (is_running_.load(std::memory_order_acquire)) {
        auto pumped_time = event_pump_.Pump(kPumpTime, active_notifications);
        for (const auto& item : active_notifications) {
            item.first->HandleEvent(pumped_time, item.second);
        }

        ProcessPendingTasks();

        active_notifications.clear();
    }
}

void EventLoop::Quit()
{
    is_running_.store(false, std::memory_order_release);

    if (!BelongsToCurrentThread()) {
        event_pump_.Wakeup();
    }
}

// static
EventLoop* EventLoop::current() noexcept
{
    return tls_loop_in_thread;
}

void EventLoop::RunTask(Task task)
{
    if (BelongsToCurrentThread()) {
        task();
    } else {
        QueueTask(std::move(task));
    }
}

void EventLoop::QueueTask(Task task)
{
    {
        std::lock_guard<std::mutex> lock(task_queue_mutex_);
        task_queue_.push_back(std::move(task));
    }

    if (!BelongsToCurrentThread() || executing_pending_task_) {
        event_pump_.Wakeup();
    }
}

void EventLoop::RegisterNotifier(Notifier* notifier)
{
    event_pump_.RegisterNotifier(notifier);
}

void EventLoop::UnregisterNotifier(Notifier* notifier)
{
    event_pump_.UnregisterNotifier(notifier);
}

void EventLoop::ProcessPendingTasks()
{
    executing_pending_task_ = true;
    ON_SCOPE_EXIT { executing_pending_task_ = false; };

    decltype(task_queue_) pending_tasks;

    {
        std::lock_guard<std::mutex> lock(task_queue_mutex_);
        pending_tasks.swap(task_queue_);
    }

    for (auto& task : pending_tasks) {
        task();
    }
}

}   // namespace ezio
