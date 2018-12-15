
set(chat_server_SRCS
    chat_server_main.cpp
    chat_server.cpp
    data_codec.cpp
)

add_executable(chat-server ${chat_server_SRCS})

target_link_libraries(chat-server PRIVATE ezio)

set(chat_client_SRCS
    chat_client_main.cpp
    chat_client.cpp
    data_codec.cpp
)

add_executable(chat-client ${chat_client_SRCS})

target_link_libraries(chat-client PRIVATE ezio)
