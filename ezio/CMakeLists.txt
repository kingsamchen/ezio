
set(ezio_SRCS
    ignore_sigpipe.cpp
)

add_library(ezio ${ezio_SRCS})

target_link_libraries(ezio PRIVATE kbase)

target_include_directories(ezio
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../
)
