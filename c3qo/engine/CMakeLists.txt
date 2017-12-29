# Build manager as a static library
set(SOURCES_MANAGER src/manager.c)


add_library(manager STATIC ${SOURCES_MANAGER})


target_include_directories(manager PUBLIC ${C3QO_BLOCK_INCLUDE_DIR})
target_include_directories(manager PUBLIC ${C3QO_LOGGER_INCLUDE_DIR})
target_include_directories(manager PUBLIC ${C3QO_MANAGER_INCLUDE_DIR})


target_link_libraries(manager hello)
target_link_libraries(manager goodbye)
target_link_libraries(manager inotify_sb)
target_link_libraries(manager client_us_asnb)
target_link_libraries(manager server_us_asnb)


# Build c3qo main executable
set(SOURCES src/main.c)


add_executable(c3qo ${SOURCES})


target_include_directories(c3qo PUBLIC ${C3QO_LOGGER_INCLUDE_DIR})
target_include_directories(c3qo PUBLIC ${C3QO_MANAGER_INCLUDE_DIR})


target_link_libraries(c3qo manager)


# Build TU for manager
set(SOURCES_TU_MANAGER test/tu_manager.cpp)


add_executable(tu_manager ${SOURCES_TU_MANAGER})


target_include_directories(tu_manager PUBLIC ${C3QO_LOGGER_INCLUDE_DIR})
target_include_directories(tu_manager PUBLIC ${C3QO_MANAGER_INCLUDE_DIR})


target_link_libraries(tu_manager manager)
target_link_libraries(tu_manager gtest)
target_link_libraries(tu_manager gtest_main)


