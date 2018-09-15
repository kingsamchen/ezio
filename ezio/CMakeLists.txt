
set(ezio_SRCS
    event_loop.cpp
    event_pump.cpp
    event_pump_impl_posix.cpp
    ignore_sigpipe.cpp
    io_service_context.cpp
    notifier.cpp
    notifier_posix.cpp
)

add_library(ezio ${ezio_SRCS})

target_link_libraries(ezio PRIVATE kbase)

target_include_directories(ezio
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../
)
