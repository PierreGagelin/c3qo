

# Build manager library
set(SOURCES_MANAGER)
set(SOURCES_MANAGER ${SOURCES_MANAGER} src/manager.cpp)
set(SOURCES_MANAGER ${SOURCES_MANAGER} src/block.cpp)
set(SOURCES_MANAGER ${SOURCES_MANAGER} src/manager_bk.cpp)
set(SOURCES_MANAGER ${SOURCES_MANAGER} src/manager_fd.cpp)
set(SOURCES_MANAGER ${SOURCES_MANAGER} src/manager_tm.cpp)

add_library(manager SHARED ${SOURCES_MANAGER})

C3QO_TARGET_INCLUDE(manager)
C3QO_TARGET_COMPILE_FLAGS(manager)
C3QO_TARGET_LINK_FLAGS(manager)

target_link_libraries(manager PUBLIC logger)
target_link_libraries(manager PUBLIC socket)


# Build c3qo main executable
set(SOURCES src/main.cpp)

add_executable(c3qo ${SOURCES})

target_compile_options(c3qo PRIVATE "${C3QO_COMPILE_FLAGS}")
C3QO_TARGET_LINK_FLAGS(c3qo)

target_link_libraries(c3qo manager)
target_link_libraries(c3qo hello)
target_link_libraries(c3qo socket_us_nb)
target_link_libraries(c3qo zmq_pair)
target_link_libraries(c3qo project_euler)

# Build network CLI executable
set(SOURCES_NETWORK_CLI src/network_cli.cpp)

add_executable(ncli ${SOURCES_NETWORK_CLI})

target_compile_options(ncli PRIVATE "${C3QO_COMPILE_FLAGS}")
C3QO_TARGET_LINK_FLAGS(ncli)

target_link_libraries(ncli PRIVATE logger)
target_link_libraries(ncli PRIVATE socket)


# Build TU for manager_bk
set(SOURCES_TU_MANAGER_BK test/tu_manager_bk.cpp)

add_executable(tu_manager_bk ${SOURCES_TU_MANAGER_BK})

C3QO_TARGET_LINK_FLAGS(tu_manager_bk)

add_test(NAME tu_manager_bk COMMAND tu_manager_bk)

target_link_libraries(tu_manager_bk manager)
target_link_libraries(tu_manager_bk hello)
target_link_libraries(tu_manager_bk socket_us_nb)
target_link_libraries(tu_manager_bk gtest)
target_link_libraries(tu_manager_bk gtest_main)


# Build TU for manager_fd
set(SOURCES_TU_MANAGER_FD test/tu_manager_fd.cpp)

add_executable(tu_manager_fd ${SOURCES_TU_MANAGER_FD})

C3QO_TARGET_LINK_FLAGS(tu_manager_fd)

add_test(NAME tu_manager_fd COMMAND tu_manager_fd)

target_link_libraries(tu_manager_fd manager)
target_link_libraries(tu_manager_fd gtest)
target_link_libraries(tu_manager_fd gtest_main)


# Build TU for manager_tm
set(SOURCES_TU_MANAGER_TM test/tu_manager_tm.cpp)

add_executable(tu_manager_tm ${SOURCES_TU_MANAGER_TM})

C3QO_TARGET_LINK_FLAGS(tu_manager_tm)

add_test(NAME tu_manager_tm COMMAND tu_manager_tm)

target_link_libraries(tu_manager_tm manager)
target_link_libraries(tu_manager_tm gtest)
target_link_libraries(tu_manager_tm gtest_main)


# Build TU for performances
set(SOURCES_TU_PERF test/tu_perf.cpp)

add_executable(tu_perf ${SOURCES_TU_PERF})

C3QO_TARGET_LINK_FLAGS(tu_perf)

add_test(NAME tu_perf COMMAND tu_perf)

target_link_libraries(tu_perf manager)
target_link_libraries(tu_perf hello)
target_link_libraries(tu_perf gtest)
target_link_libraries(tu_perf gtest_main)

