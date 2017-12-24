cmake_minimum_required(VERSION 3.0)
project(socket)

set(SOURCES src/socket.c)

add_library(socket MODULE ${SOURCES})

target_include_directories(socket PUBLIC ${C3QO_SOCKET_INCLUDE_DIR})
