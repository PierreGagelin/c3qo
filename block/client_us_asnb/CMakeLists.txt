cmake_minimum_required(VERSION 3.0)
project(client_us_asnb)

set(SOURCES common/src/client_us_asnb.c)

add_library(client_us_asnb MODULE ${SOURCES})

target_include_directories(client_us_asnb PUBLIC ${C3QO_BLOCK_INCLUDE_DIR})
target_include_directories(client_us_asnb PUBLIC ${C3QO_SIGNAL_INCLUDE_DIR})
target_include_directories(client_us_asnb PUBLIC ${C3QO_SOCKET_INCLUDE_DIR})
