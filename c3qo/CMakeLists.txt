

# Build manager library
set(SOURCES_MANAGER)
set(SOURCES_MANAGER ${SOURCES_MANAGER} src/manager.cpp)
set(SOURCES_MANAGER ${SOURCES_MANAGER} src/block.cpp)
set(SOURCES_MANAGER ${SOURCES_MANAGER} src/manager_bk.cpp)
set(SOURCES_MANAGER ${SOURCES_MANAGER} src/manager_fd.cpp)
set(SOURCES_MANAGER ${SOURCES_MANAGER} src/manager_tm.cpp)

c3qo_add_library(manager "${SOURCES_MANAGER}")
c3qo_target_include(manager)
target_link_libraries(manager logger)
target_link_libraries(manager hello)
target_link_libraries(manager socket_us_nb)
target_link_libraries(manager zmq_pair)
target_link_libraries(manager project_euler)
target_link_libraries(manager trans_pb)


# Build c3qo main executable
c3qo_add_executable(c3qo src/main.cpp)
target_link_libraries(c3qo manager)


# Build network CLI executable
c3qo_add_executable(ncli src/network_cli.cpp)
target_link_libraries(ncli socket)


if (NOT ${GTEST_DISABLE})
    # Build TU for manager_bk
    c3qo_add_test(tu_manager_bk test/tu_manager_bk.cpp)
    target_link_libraries(tu_manager_bk manager)

    # Build TU for manager_fd
    c3qo_add_test(tu_manager_fd test/tu_manager_fd.cpp)
    target_link_libraries(tu_manager_fd manager)

    # Build TU for manager_tm
    c3qo_add_test(tu_manager_tm test/tu_manager_tm.cpp)
    target_link_libraries(tu_manager_tm manager)

    # Build TU for performances
    c3qo_add_test(tu_perf test/tu_perf.cpp)
    target_link_libraries(tu_perf manager)
endif ()

