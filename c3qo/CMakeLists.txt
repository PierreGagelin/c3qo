

# Build manager as a static library
set(SOURCES_MANAGER)
set(SOURCES_MANAGER ${SOURCES_MANAGER} src/block.cpp)
set(SOURCES_MANAGER ${SOURCES_MANAGER} src/manager_bk.cpp)
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


# Build TU for managers
set(SOURCES_TU_MANAGER_BK test/tu_manager_bk.cpp)
set(SOURCES_TU_MANAGER_FD test/tu_manager_fd.cpp)
set(SOURCES_TU_MANAGER_TM test/tu_manager_bk.cpp)

add_executable(tu_manager_bk ${SOURCES_TU_MANAGER_BK})
add_executable(tu_manager_fd ${SOURCES_TU_MANAGER_FD})
add_executable(tu_manager_tm ${SOURCES_TU_MANAGER_TM})
add_test(NAME tu_manager_bk COMMAND tu_manager_bk)
add_test(NAME tu_manager_fd COMMAND tu_manager_fd)
add_test(NAME tu_manager_tm COMMAND tu_manager_tm)

target_include_directories(tu_manager_bk PUBLIC ${UTILS_INCLUDE_DIR})
target_include_directories(tu_manager_fd PUBLIC ${UTILS_INCLUDE_DIR})
target_include_directories(tu_manager_tm PUBLIC ${UTILS_INCLUDE_DIR})
target_include_directories(tu_manager_bk PUBLIC ${C3QO_INCLUDE_DIR})
target_include_directories(tu_manager_fd PUBLIC ${C3QO_INCLUDE_DIR})
target_include_directories(tu_manager_tm PUBLIC ${C3QO_INCLUDE_DIR})

target_link_libraries(tu_manager_bk manager)
target_link_libraries(tu_manager_fd manager)
target_link_libraries(tu_manager_tm manager)
target_link_libraries(tu_manager_bk logger)
target_link_libraries(tu_manager_fd logger)
target_link_libraries(tu_manager_tm logger)
target_link_libraries(tu_manager_bk gtest)
target_link_libraries(tu_manager_fd gtest)
target_link_libraries(tu_manager_tm gtest)
target_link_libraries(tu_manager_bk gtest_main)
target_link_libraries(tu_manager_fd gtest_main)
target_link_libraries(tu_manager_tm gtest_main)


# Build TU for performances
set(SOURCES_TU_PERF test/tu_perf.cpp)

add_executable(tu_perf ${SOURCES_TU_PERF})
add_test(NAME tu_perf COMMAND tu_perf)

target_include_directories(tu_perf PUBLIC ${UTILS_INCLUDE_DIR})
target_include_directories(tu_perf PUBLIC ${C3QO_INCLUDE_DIR})

target_link_libraries(tu_perf manager)
target_link_libraries(tu_perf logger)
target_link_libraries(tu_perf gtest)
target_link_libraries(tu_perf gtest_main)

