/*
 @ 0xCCCCCCCC
*/

#ifndef EZIO_THIS_THREAD_H_
#define EZIO_THIS_THREAD_H_

#include "kbase/basic_macros.h"

#if defined(OS_POSIX)
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>
#elif defined(OS_WIN)
#include <Windows.h>
#endif

namespace ezio {
namespace this_thread {

#if defined(OS_POSIX)
using ThreadID = pid_t;
#elif defined(OS_WIN)
using ThreadID = DWORD;
#endif

inline ThreadID GetThreadID()
{
#if defined(OS_POSIX)
    return static_cast<ThreadID>(syscall(SYS_gettid));
#elif defined(OS_WIN)
    return GetCurrentThreadId();
#endif
}

}   // namespace this_thread
}   // namespace ezio

#endif  // EZIO_THIS_THREAD_H_
