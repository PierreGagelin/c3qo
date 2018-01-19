

# Build server_us_nb as a static library
set(SOURCES src/server_us_nb.c)


add_library(server_us_nb STATIC ${SOURCES})


target_include_directories(server_us_nb PUBLIC ${C3QO_BLOCK_INCLUDE_DIR})
target_include_directories(server_us_nb PUBLIC ${C3QO_LOGGER_INCLUDE_DIR})
target_include_directories(server_us_nb PUBLIC ${C3QO_MANAGER_INCLUDE_DIR})
target_include_directories(server_us_nb PUBLIC ${C3QO_SOCKET_INCLUDE_DIR})


target_link_libraries(server_us_nb manager)
target_link_libraries(server_us_nb socket)


# Build TU for socket_us_nb
set(SOURCES_TU_SOCKET_US_NB test/tu_socket_us_nb.cpp)


add_executable(tu_socket_us_nb ${SOURCES_TU_SOCKET_US_NB})
add_test(NAME tu_socket_us_nb COMMAND tu_socket_us_nb)


target_include_directories(tu_socket_us_nb PUBLIC ${C3QO_LOGGER_INCLUDE_DIR})
target_include_directories(tu_socket_us_nb PUBLIC ${C3QO_MANAGER_INCLUDE_DIR})


target_link_libraries(tu_socket_us_nb client_us_nb) # client
target_link_libraries(tu_socket_us_nb server_us_nb) # server
target_link_libraries(tu_socket_us_nb logger)       # logs
target_link_libraries(tu_socket_us_nb gtest)        # gtest library
target_link_libraries(tu_socket_us_nb gtest_main)   # gtest automatic main


