

set(SOURCES src/server_us_nb.c)


add_library(server_us_nb STATIC ${SOURCES})


target_include_directories(server_us_nb PUBLIC ${C3QO_BLOCK_INCLUDE_DIR})
target_include_directories(server_us_nb PUBLIC ${C3QO_LOGGER_INCLUDE_DIR})
target_include_directories(server_us_nb PUBLIC ${C3QO_MANAGER_INCLUDE_DIR})
target_include_directories(server_us_nb PUBLIC ${C3QO_SIGNAL_INCLUDE_DIR})
target_include_directories(server_us_nb PUBLIC ${C3QO_SOCKET_INCLUDE_DIR})


target_link_libraries(server_us_nb manager)
target_link_libraries(server_us_nb signal)
target_link_libraries(server_us_nb socket)


