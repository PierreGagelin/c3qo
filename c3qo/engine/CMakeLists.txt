set(SOURCES src/main.c src/manager.c src/manager.h)


add_executable(c3qo ${SOURCES})


target_include_directories(c3qo PUBLIC ${C3QO_BLOCK_INCLUDE_DIR})
target_include_directories(c3qo PUBLIC ${C3QO_LOGGER_INCLUDE_DIR})


target_link_libraries(c3qo hello)
target_link_libraries(c3qo goodbye)
target_link_libraries(c3qo inotify_sb)
target_link_libraries(c3qo client_us_asnb)
target_link_libraries(c3qo server_us_asnb)


