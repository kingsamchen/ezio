/*
 @ 0xCCCCCCCC
*/

#include "ezio/timer_queue.h"

#include <functional>

#include "kbase/error_exception_util.h"
#include "kbase/scope_guard.h"

#include "ezio/event_loop.h"

namespace ezio {

TimerQueue::TimerQueue(EventLoop* loop)
    : loop_(loop),
      processing_expired_timers_(false)
{}

TimerID TimerQueue::AddTimer(Timer::TickEventHandler handler, TimePoint when, TimeDuration interval)
{
    auto new_timer = std::make_unique<Timer>(std::move(handler), when, interval);
    TimerID timer_id(new_timer.get());

    loop_->RunTask(std::bind(&TimerQueue::AddTimerInLoop, this, new_timer.release()));

    return timer_id;
}

void TimerQueue::AddTimerInLoop(Timer* new_timer)
{
    ENSURE(CHECK, loop_->BelongsToCurrentThread()).Require();

    auto new_earliest = Insert(std::unique_ptr<Timer>(new_timer));
    if (new_earliest) {
        loop_->Wakeup();
    }
}

void TimerQueue::Cancel(TimerID timer_id)
{
    loop_->RunTask(std::bind(&TimerQueue::CancelInLoop, this, timer_id));
}

void TimerQueue::CancelInLoop(TimerID timer_id)
{
    ENSURE(CHECK, loop_->BelongsToCurrentThread()).Require();

    auto range = timers_.equal_range(timer_id.timer()->expiration());
    auto it = std::find_if(range.first, range.second, [timer_id](const auto& timer) {
        return timer_id.timer() == timer.second.get();
    });

    // The function maybe called by a repeating timer to cancel itself.
    if (it == range.second) {
        if (processing_expired_timers_) {
            canceling_timers_.insert(timer_id.timer());
        }

        return;
    }

    timers_.erase(it);
}

void TimerQueue::ProcessExpiredTimers(TimePoint now)
{
    ENSURE(CHECK, loop_->BelongsToCurrentThread()).Require();

    auto end = timers_.upper_bound(now);
    ENSURE(CHECK, end == timers_.end() || now < end->first).Require();

    std::vector<std::unique_ptr<Timer>> expired;
    for (auto it = timers_.begin(); it != end; ++it) {
        expired.push_back(std::move(it->second));
    }

    timers_.erase(timers_.begin(), end);

    // Handle timer ticks.
    {
        canceling_timers_.clear();

        processing_expired_timers_ = true;
        ON_SCOPE_EXIT { processing_expired_timers_ = false; };

        for (const auto& timer : expired) {
            timer->Tick();
        }
    }

    // Restart timers or remove canceled timers.
    for (auto& timer : expired) {
        if (timer->is_repeating() && canceling_timers_.count(timer.get()) == 0) {
            timer->Restart(now);
            Insert(std::move(timer));
        }
    }
}

bool TimerQueue::Insert(std::unique_ptr<Timer> new_timer)
{
    auto when = new_timer->expiration();
    auto it = timers_.cbegin();

    bool earliest_changed = (it == timers_.cend() || when < it->first);

    timers_.emplace(std::make_pair(when, std::move(new_timer)));

    return earliest_changed;
}

std::pair<bool, TimePoint> TimerQueue::next_expiration() const
{
    ENSURE(CHECK, loop_->BelongsToCurrentThread()).Require();

    if (timers_.empty()) {
        return {false, TimePoint::max()};
    }

    return {true, timers_.begin()->first};
}

}   // namespace ezio
