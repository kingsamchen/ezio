/*
 @ 0xCCCCCCCC
*/

#ifndef EZIO_TIMER_ID_H_
#define EZIO_TIMER_ID_H_

#include "kbase/basic_macros.h"

namespace ezio {

class Timer;

class TimerID {
public:
    explicit TimerID(Timer* timer)
        : timer_(timer)
    {}

    DEFAULT_COPY(TimerID);

    DEFAULT_MOVE(TimerID);

    Timer* timer() const noexcept
    {
        return timer_;
    }

private:
    Timer* timer_;
};

}   // namespace ezio

#endif  // EZIO_TIMER_ID_H_
