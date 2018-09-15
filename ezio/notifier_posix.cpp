/*
 @ 0xCCCCCCCC
*/

#include "ezio/notifier.h"

namespace ezio {

void Notifier::HandleEvent(TimePoint receive_time, IOContext io_ctx) const
{
    auto events = io_ctx.event;
    constexpr auto details = io_ctx.ToDetails();

    if ((events & EPOLLHUP) && !(events & EPOLLIN)) {
        if (on_close_) {
            on_close_();
        }
    }

    if (events & EPOLLERR) {
        if (on_error_) {
            on_error_();
        }
    }

    if (events & (EPOLLIN | EPOLLPRI | EPOLLRDHUP)) {
        if (on_read_) {
            on_read_(receive_time, details);
        }
    }

    if (events & EPOLLOUT) {
        if (on_write_) {
            on_write_(details);
        }
    }
}

}   // namespace ezio
