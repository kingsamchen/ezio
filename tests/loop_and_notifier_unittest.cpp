/*
 @ 0xCCCCCCCC
*/

#include "catch2/catch.hpp"

#include "ezio/event_loop.h"

#include <cstdio>
#include <thread>

#include "kbase/at_exit_manager.h"

#include "ezio/chrono_utils.h"
#include "ezio/io_service_context.h"

namespace ezio {

TEST_CASE("EventLoop and Notifier are two fundamental building blocks", "[MainLoop]")
{
    kbase::AtExitManager exit_manager;
    IOServiceContext::Init();

    SECTION("run a loop and quit from the loop")
    {
        EventLoop loop;

        std::thread th([&loop] {
            printf("worker is sleeping\n");
            std::this_thread::sleep_for(std::chrono::seconds(3));
            printf("Quit main loop from worker thread\n");
            loop.Quit();
        });

        printf("Main loop runs\n");
        loop.Run();
        printf("Main loop ends\n");

        th.join();
    }

    SECTION("queue tasks and run tasks")
    {
        EventLoop loop;

        std::thread th([&loop] {
            // Queuing and running task both wake up the loop.
            std::this_thread::sleep_for(std::chrono::seconds(3));
            printf("queue a task from thread %u\n", this_thread::GetThreadID());
            loop.QueueTask([&loop] {
                printf("queuing\n");

                loop.RunTask([] {
                    printf("execute task on thread %u\n...\n", this_thread::GetThreadID());
                });

                // Quit should be executed sequentially after previous lambda.
                printf("quit\n");
                loop.Quit();
            });
        });

        printf("running loop on thread %u\n", this_thread::GetThreadID());

        loop.Run();

        th.join();
    }

    SECTION("runs a timed task")
    {
        EventLoop loop;

        loop.RunTaskAfter([] {
            printf("The timed task is executing\n");
            EventLoop::current()->Quit();
        }, std::chrono::seconds(3));

        printf("EventLoop is running!\n");

        loop.Run();
    }

    SECTION("timed tasks are ordered by their expiration")
    {
        EventLoop loop;

        loop.RunTaskAfter([] {
            printf("Task with 5s delayed\n");
            EventLoop::current()->RunTaskAfter([] {
                printf("Wait for another 2s to quit\n");
                EventLoop::current()->Quit();
            }, std::chrono::seconds(2));
        }, std::chrono::seconds(5));

        std::thread th([&loop] {
            loop.RunTaskAt([] {
                printf("Task with 3s delayed\n");
            }, ToTimePoint(std::chrono::system_clock::now()) + std::chrono::seconds(3));
        });

        printf("EventLoop is running!\n");

        loop.Run();

        th.join();
    }

    SECTION("cancel a timer")
    {
        EventLoop loop;

        auto t1 = loop.RunTaskAfter([] {
            printf("Task with 3s dealyed\n");
        }, std::chrono::seconds(3));

        loop.RunTaskAfter([] {
            printf("quit\n");
            EventLoop::current()->Quit();
        }, std::chrono::seconds(5));

        std::thread th([&loop, t1] {
            printf("worker is zzzz\n");
            std::this_thread::sleep_for(std::chrono::seconds(2));
            loop.CancelTimedTask(t1);
            printf("first timed task is canceled!\n");
        });

        printf("EventLoop is running!\n");

        loop.Run();

        th.join();
    }

    SECTION("cancel a repeating timer")
    {
        EventLoop loop;

        int repeated_count = 0;
        // Yuck. Don't use this hack in production code.
        TimerID timer(nullptr);
        timer = loop.RunTaskEvery([&repeated_count, &timer] {
            printf("run for %d time\n", ++repeated_count);
            if (repeated_count >= 5) {
                EventLoop::current()->CancelTimedTask(timer);
                EventLoop::current()->Quit();
            }
        }, std::chrono::seconds(1));

        printf("EventLoop is running!\n");

        loop.Run();
    }
}

}   // namespace ezio
