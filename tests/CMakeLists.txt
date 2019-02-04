
include(${EZIO_DEPS_DIR}/FetchCatch2.cmake)

set(tests_SRCS
  main.cpp
  acceptor_unittest.cpp
  buffer_unittest.cpp
  connector_and_tcpclient.cpp
  endian_utils_unittest.cpp
  io_service_context_unittest.cpp
  loop_and_notifier_unittest.cpp
  scoped_socket_unittest.cpp
  socket_address_unittest.cpp
  tcp_server_and_connection.cpp
  thread_and_worker_pool.cpp
)

if(WIN32)
  list(APPEND tests_SRCS
    winsock_context_unittest.cpp
  )
else(UNIX)
  list(APPEND tests_SRCS
    ignore_sigpipe_unittest.cpp
  )
endif()

source_group("tests" FILES ${tests_SRCS})

add_executable(ezio_test ${tests_SRCS})

apply_common_compile_properties_to_target(ezio_test)

set_target_properties(ezio_test PROPERTIES
  COTIRE_CXX_PREFIX_HEADER_INIT "${EZIO_PCH_HEADER}"
)

cotire(ezio_test)

target_link_libraries(ezio_test
  PRIVATE
    ezio
    kbase
    Catch2
)
