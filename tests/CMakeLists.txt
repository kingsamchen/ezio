
set(test_SRCS
    main.cpp
    endian_utils_unittest.cpp
    ignore_sigpipe_unittest.cpp
    io_service_context_unittest.cpp
    loop_and_notifier_unittest.cpp
    scoped_socket_unittest.cpp
)

add_executable(ezio_test ${test_SRCS})

target_include_directories(ezio_test
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../KBase/test/Catch2/single_include
)

target_link_libraries(ezio_test PUBLIC ezio kbase pthread)
