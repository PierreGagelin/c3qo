set(SOURCES src/server_us_asnb.c)


add_library(server_us_asnb STATIC ${SOURCES})


target_include_directories(server_us_asnb PUBLIC ${C3QO_BLOCK_INCLUDE_DIR})
target_include_directories(server_us_asnb PUBLIC ${C3QO_SIGNAL_INCLUDE_DIR})
target_include_directories(server_us_asnb PUBLIC ${C3QO_SOCKET_INCLUDE_DIR})


