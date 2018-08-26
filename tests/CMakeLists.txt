
set(test_SRCS
    main.cpp
)

add_executable(ezio_test ${test_SRCS})

target_include_directories(ezio_test
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../KBase/test/Catch2/single_include
)

target_link_libraries(ezio_test PUBLIC ezio kbase pthread)
