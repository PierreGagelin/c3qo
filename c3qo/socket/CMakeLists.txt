set(SOURCES src/socket.c)

add_library(socket STATIC ${SOURCES})

target_include_directories(socket PUBLIC ${C3QO_SOCKET_INCLUDE_DIR})
target_include_directories(socket PUBLIC ${C3QO_LOGGER_INCLUDE_DIR})


