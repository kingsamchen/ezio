
add_executable(echo-server echo_server.cpp)

apply_common_compile_properties_to_target(echo-server)

set_target_properties(echo-server PROPERTIES
  FOLDER examples/echo
)

target_link_libraries(echo-server
  PRIVATE
    ezio
    kbase
)

add_executable(echo-client echo_client.cpp)

apply_common_compile_properties_to_target(echo-client)

set_target_properties(echo-client PROPERTIES
  FOLDER examples/echo
)

target_link_libraries(echo-client
  PRIVATE
    ezio
    kbase
)
