/*
 @ 0xCCCCCCCC
*/

#include "catch2/catch.hpp"

#include "ezio/thread.h"

#include <cstdio>
#include <string>

#include "kbase/at_exit_manager.h"

#include "ezio/event_loop.h"
#include "ezio/io_service_context.h"
#include "ezio/this_thread.h"
#include "ezio/worker_pool.h"

namespace ezio {

TEST_CASE("Thread is native thread running EventLoop", "[Thread]")
{
    printf("Main thread %u\n", this_thread::GetID());

    Thread th("Worker-101");
    th.event_loop()->RunTask([&th] {
        printf("%s thread %u\n", th.name().c_str(), this_thread::GetID());
    });

    th.event_loop()->RunTaskAfter([] {
        printf("timed beep!\n");
    }, std::chrono::seconds(3));

    printf("Main thread gets sleep for 5s\n");
    std::this_thread::sleep_for(std::chrono::seconds(5));
}

TEST_CASE("EventLoop of a Thread quits from outside", "[Thread]")
{
    printf("Main thread %u\n", this_thread::GetID());

    Thread th("Worker-101");
    th.event_loop()->RunTask([&th] {
        printf("%s thread %u\n", th.name().c_str(), this_thread::GetID());
    });

    // Make sure worker thread runs before we call Quit().
    std::this_thread::sleep_for(std::chrono::seconds(2));

    // Danger!!
    // DO NOT DO THIS IN PRODUCTION CODE.
    th.event_loop()->Quit();

    // Simulate busy working on main thread, ensure Quit() takes effect before `th` goes
    // destruction.
    std::this_thread::sleep_for(std::chrono::seconds(2));
}

TEST_CASE("WorkerPool runs several worker threads", "[WorkerPool]")
{
    kbase::AtExitManager exit_manager;
    IOServiceContext::Init();

    // We don't even need to run the main-loop.
    EventLoop main;

    WorkerPool pool(&main, 3, "Batman");

    for (int i = 0; i < 3; ++i) {
        pool.GetNextEventLoop()->RunTask([] {
            printf("%s %u\n", this_thread::GetName(), this_thread::GetID());
        });
    }

    printf("%s %u\n", this_thread::GetName(), this_thread::GetID());
}

}   // namespace ezio
