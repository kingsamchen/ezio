
set(socks4a_SRCS
  main.cpp
  socks_proxy.cpp
  tunnel.cpp
)

set(socks4a_HEADERS
  socks_proxy.h
  tunnel.h
)

set(socks4a_FILES ${socks4a_SRCS} ${socks4a_HEADERS})

source_group("socks4a" FILES ${socks4a_FILES})

add_executable(socks4a-proxy ${socks4a_FILES})

apply_common_compile_properties_to_target(socks4a-proxy)

set_target_properties(socks4a-proxy PROPERTIES
    FOLDER examples/socks-4a
)

target_link_libraries(socks4a-proxy
  PRIVATE
    ezio
    kbase
)
