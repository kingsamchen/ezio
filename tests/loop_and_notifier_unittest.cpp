/*
 @ 0xCCCCCCCC
*/

#include "catch2/catch.hpp"

#include "ezio/event_loop.h"

#include <cstdio>
#include <thread>

#include "kbase/at_exit_manager.h"

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
}

}   // namespace ezio
