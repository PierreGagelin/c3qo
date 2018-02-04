

# Build manager as a static library
set(SOURCES_MANAGER                    src/manager_bk.cpp)
set(SOURCES_MANAGER ${SOURCES_MANAGER} src/manager_fd.cpp)
set(SOURCES_MANAGER ${SOURCES_MANAGER} src/manager_tm.cpp)


add_library(manager STATIC ${SOURCES_MANAGER})


target_include_directories(manager PUBLIC ${C3QO_INCLUDE_DIR})
target_include_directories(manager PUBLIC ${UTILS_INCLUDE_DIR})


target_link_libraries(manager hello)
target_link_libraries(manager client_us_nb)
target_link_libraries(manager server_us_nb)
target_link_libraries(manager logger)


# Build c3qo main executable
set(SOURCES src/main.cpp)


add_executable(c3qo ${SOURCES})


target_include_directories(c3qo PUBLIC ${UTILS_INCLUDE_DIR})
target_include_directories(c3qo PUBLIC ${C3QO_INCLUDE_DIR})


target_link_libraries(c3qo manager)


# Build TU for manager
set(SOURCES_TU_MANAGER test/tu_manager.cpp)


add_executable(tu_manager ${SOURCES_TU_MANAGER})
add_test(NAME tu_manager COMMAND tu_manager)


target_include_directories(tu_manager PUBLIC ${UTILS_INCLUDE_DIR})
target_include_directories(tu_manager PUBLIC ${C3QO_INCLUDE_DIR})


target_link_libraries(tu_manager manager)
target_link_libraries(tu_manager logger)
target_link_libraries(tu_manager gtest)
target_link_libraries(tu_manager gtest_main)


