
include(${EZIO_DEPS_DIR}/FetchKBase.cmake)

set(ezio_SRCS
  acceptor.cpp
  buffer.cpp
  connector_base.cpp
  event_loop.cpp
  event_pump.cpp
  io_service_context.cpp
  notifier.cpp
  socket_address.cpp
  socket_utils.cpp
  tcp_client.cpp
  tcp_connection.cpp
  tcp_server.cpp
  this_thread.cpp
  thread.cpp
  timer.cpp
  timer_queue.cpp
  worker_pool.cpp
)

if(WIN32)
  list(APPEND ezio_SRCS
    acceptor_win.cpp
    connector_win.cpp
    event_pump_impl_win.cpp
    notifier_win.cpp
    socket_utils_win.cpp
    tcp_connection_win.cpp
    winsock_context.cpp
  )
elseif(UNIX)
  list(APPEND ezio_SRCS
    acceptor_posix.cpp
    buffer_posix.cpp
    connector_posix.cpp
    event_pump_impl_posix.cpp
    ignore_sigpipe.cpp
    notifier_posix.cpp
    socket_utils_posix.cpp
    tcp_connection_posix.cpp
  )
endif()

set(ezio_HEADERS
  acceptor.h
  buffer.h
  chrono_utils.h
  common_event_handlers.h
  connector_base.h
  connector.h
  endian_utils.h
  event_loop.h
  event_pump.h
  io_context.h
  io_service_context.h
  notifier.h
  scoped_socket.h
  socket_address.h
  socket_utils.h
  tcp_client.h
  tcp_connection.h
  tcp_server.h
  this_thread.h
  thread.h
  timer_id.h
  timer_queue.h
  timer.h
  worker_pool.h
)

if(WIN32)
  list(APPEND ezio_HEADERS
    connector_win.h
    event_pump_impl_win.h
    winsock_context.h
  )
elseif(UNIX)
  list(APPEND ezio_HEADERS
    connector_posix.h
    event_pump_impl_posix.h
    ignore_sigpipe.h
  )
endif()

set(ezio_FILES ${ezio_HEADERS} ${ezio_SRCS})

source_group("ezio" FILES ${ezio_FILES})

add_library(ezio ${ezio_FILES})

apply_common_compile_properties_to_target(ezio)

if(MSVC)
  enable_msvc_static_analysis_for_target(ezio)
endif()

target_include_directories(ezio
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../
)

target_compile_definitions(ezio
  PUBLIC WIN32_LEAN_AND_MEAN
)

set_target_properties(ezio PROPERTIES
  COTIRE_CXX_PREFIX_HEADER_INIT "${EZIO_PCH_HEADER}"
)

cotire(ezio)

target_link_libraries(ezio
  PRIVATE kbase
)

if(UNIX)
  target_link_libraries(ezio
    INTERFACE pthread)
endif()
