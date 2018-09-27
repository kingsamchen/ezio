
set(ezio_SRCS
    acceptor.cpp
    acceptor_posix.cpp
    buffer.cpp
    buffer_posix.cpp
    event_loop.cpp
    event_pump.cpp
    event_pump_impl_posix.cpp
    ignore_sigpipe.cpp
    io_service_context.cpp
    notifier.cpp
    notifier_posix.cpp
    socket_address.cpp
    socket_utils.cpp
    socket_utils_posix.cpp
    timer.cpp
    timer_queue.cpp
)

add_library(ezio ${ezio_SRCS})

target_link_libraries(ezio PRIVATE kbase)

target_include_directories(ezio
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../
)
