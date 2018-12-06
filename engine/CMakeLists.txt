

# Build manager library
set(SOURCES_MANAGER)
set(SOURCES_MANAGER ${SOURCES_MANAGER} src/block.cpp)
set(SOURCES_MANAGER ${SOURCES_MANAGER} src/manager_bk.cpp)
set(SOURCES_MANAGER ${SOURCES_MANAGER} src/manager_fd.cpp)
set(SOURCES_MANAGER ${SOURCES_MANAGER} src/manager_tm.cpp)

c3qo_add_library(manager "${SOURCES_MANAGER}")
target_include_directories(manager PUBLIC include/)
target_link_libraries(manager logger)
target_link_libraries(manager socket)

if (${C3QO_TEST})
    # Build TU for block
    c3qo_add_test(tu_block test/tu_block.cpp)
    target_link_libraries(tu_block manager)
    target_link_libraries(tu_block hello)

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
    target_link_libraries(tu_perf hello)
endif()
