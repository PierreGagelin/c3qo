

set(SOURCES src/client_us_nb.c)


add_library(client_us_nb STATIC ${SOURCES})


target_include_directories(client_us_nb PUBLIC ${C3QO_BLOCK_INCLUDE_DIR})
target_include_directories(client_us_nb PUBLIC ${C3QO_LOGGER_INCLUDE_DIR})
target_include_directories(client_us_nb PUBLIC ${C3QO_SIGNAL_INCLUDE_DIR})
target_include_directories(client_us_nb PUBLIC ${C3QO_SOCKET_INCLUDE_DIR})


target_link_libraries(client_us_nb signal)
target_link_libraries(client_us_nb socket)


