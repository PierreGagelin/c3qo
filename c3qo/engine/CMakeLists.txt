# Build manager as a static library
set(SOURCES_MANAGER src/manager.c)


add_library(manager STATIC ${SOURCES_MANAGER})


target_include_directories(manager PUBLIC ${C3QO_BLOCK_INCLUDE_DIR})
target_include_directories(manager PUBLIC ${C3QO_LOGGER_INCLUDE_DIR})
target_include_directories(manager PUBLIC ${C3QO_MANAGER_INCLUDE_DIR})


# Build c3qo main executable
set(SOURCES src/main.c)


add_executable(c3qo ${SOURCES})


target_include_directories(manager PUBLIC ${C3QO_MANAGER_INCLUDE_DIR})


target_link_libraries(c3qo manager)
target_link_libraries(c3qo hello)
target_link_libraries(c3qo goodbye)
target_link_libraries(c3qo inotify_sb)
target_link_libraries(c3qo client_us_asnb)
target_link_libraries(c3qo server_us_asnb)


