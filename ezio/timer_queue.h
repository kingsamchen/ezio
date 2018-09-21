/*
 @ 0xCCCCCCCC
*/

#ifndef EZIO_TIMER_QUEUE_H_
#define EZIO_TIMER_QUEUE_H_

#include <map>
#include <memory>
#include <set>
#include <utility>

#include "kbase/basic_macros.h"

#include "ezio/chrono_utils.h"
#include "ezio/timer.h"
#include "ezio/timer_id.h"

namespace ezio {

class EventLoop;

class TimerQueue {
public:
    explicit TimerQueue(EventLoop* loop);

    ~TimerQueue() = default;

    DISALLOW_COPY(TimerQueue);

    DISALLOW_MOVE(TimerQueue);

    TimerID AddTimer(Timer::TickEventHandler handler, TimePoint when, TimeDuration interval);

    void Cancel(TimerID timer_id);

    void ProcessExpiredTimers(TimePoint expiration);

    std::pair<bool, TimePoint> next_expiration() const;

private:
    // `new_timer` is an owner.
    void AddTimerInLoop(Timer* new_timer);

    bool Insert(std::unique_ptr<Timer> new_timer);

    void CancelInLoop(TimerID timer_id);

private:
    EventLoop* loop_;
    std::multimap<TimePoint, std::unique_ptr<Timer>> timers_;
    bool processing_expired_timers_;
    std::set<Timer*> canceling_timers_;
};

}   // namespace ezio

#endif  // EZIO_TIMER_QUEUE_H_
