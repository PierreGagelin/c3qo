

# Generate configuration interface
c3qo_add_library_protobuf_c(pb_config src conf)

# Build manager library
set(SOURCES_MANAGER)
set(SOURCES_MANAGER ${SOURCES_MANAGER} src/block.cpp)
set(SOURCES_MANAGER ${SOURCES_MANAGER} src/manager_bk.cpp)
set(SOURCES_MANAGER ${SOURCES_MANAGER} src/manager_conf.cpp)
set(SOURCES_MANAGER ${SOURCES_MANAGER} src/manager_fd.cpp)
set(SOURCES_MANAGER ${SOURCES_MANAGER} src/manager_tm.cpp)

c3qo_add_library(manager "${SOURCES_MANAGER}")
target_include_directories(manager PUBLIC include/)
target_link_libraries(manager dl)
target_link_libraries(manager pb_config)
target_link_libraries(manager logger)
target_link_libraries(manager socket)


# Build c3qo main executable
c3qo_add_executable(c3qo src/main.cpp)
target_link_libraries(c3qo manager)
c3qo_link_block(c3qo hello)
c3qo_link_block(c3qo trans_pb)
c3qo_link_block(c3qo zmq_pair)


# Build network CLI executable
c3qo_add_executable(ncli src/network_cli.cpp)
target_link_libraries(ncli pb_config)
target_link_libraries(ncli socket)


if (${C3QO_TEST})
    # Build TU for manager_bk
    c3qo_add_test(tu_manager_bk test/tu_manager_bk.cpp)
    target_link_libraries(tu_manager_bk manager)
    c3qo_link_block(tu_manager_bk hello)

    # Build TU for manager_conf
    c3qo_add_test(tu_manager_conf test/tu_manager_conf.cpp)
    target_link_libraries(tu_manager_conf manager)
    c3qo_link_block(tu_manager_conf hello)

    # Build TU for manager_fd
    c3qo_add_test(tu_manager_fd test/tu_manager_fd.cpp)
    target_link_libraries(tu_manager_fd manager)

    # Build TU for manager_tm
    c3qo_add_test(tu_manager_tm test/tu_manager_tm.cpp)
    target_link_libraries(tu_manager_tm manager)

    # Build TU for performances
    c3qo_add_test(tu_perf test/tu_perf.cpp)
    target_link_libraries(tu_perf manager)
    c3qo_link_block(tu_perf hello)
endif()
