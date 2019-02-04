
set(chat_server_SRCS
  chat_server_main.cpp
  chat_server.cpp
  data_codec.cpp
)

set(chat_server_HEADERS
  chat_server.h
  data_codec.h
)

add_executable(chat-server ${chat_server_SRCS} ${chat_server_HEADERS})

apply_common_compile_properties_to_target(chat-server)

set_target_properties(chat-server PROPERTIES
  FOLDER examples/chat
)

target_link_libraries(chat-server
  PRIVATE
    ezio
    kbase
)

set(chat_client_SRCS
  chat_client_main.cpp
  chat_client.cpp
  data_codec.cpp
)

set(chat_client_HEADERS
  chat_client.h
  data_codec.h
)

add_executable(chat-client ${chat_client_SRCS} ${chat_client_HEADERS})

apply_common_compile_properties_to_target(chat-client)

set_target_properties(chat-client PROPERTIES
  FOLDER examples/chat
)

target_link_libraries(chat-client
  PRIVATE
    ezio
    kbase
)
