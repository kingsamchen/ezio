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

}   // namespace ezio
