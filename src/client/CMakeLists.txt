add_library(rmBifrost_client bifrost_client.cpp
        bifrost_client_impl.cpp
        bifrost_client_impl.h
        ../utils/unix_socket.cpp
        ../utils/unix_socket.h
        ../utils/shm_channel.cpp
        ../utils/shm_channel.h)

add_library(rmBifrost::client ALIAS rmBifrost_client)