

# Build manager as a static library
## Block manager
set(SOURCES_MANAGER_BK)
set(SOURCES_MANAGER_BK ${SOURCES_MANAGER_BK} src/manager_bk.cpp)
set(SOURCES_MANAGER_BK ${SOURCES_MANAGER_BK} src/block.cpp)

add_library(manager_bk STATIC ${SOURCES_MANAGER_BK})

C3QO_TARGET_COMPILE_FLAGS(manager_bk)

### Give a platform to host the blocks
target_link_libraries(manager_bk logger)
target_link_libraries(manager_bk socket)
target_include_directories(manager_bk PUBLIC ${BLOCK_INCLUDE_DIR})
target_include_directories(manager_bk PUBLIC ${C3QO_INCLUDE_DIR})
target_include_directories(manager_bk PUBLIC ${UTILS_INCLUDE_DIR})

target_link_libraries(manager_bk hello)
target_link_libraries(manager_bk client_us_nb)
target_link_libraries(manager_bk server_us_nb)
target_link_libraries(manager_bk pub_sub)


## File descriptor manager
set(SOURCES_MANAGER_FD)
set(SOURCES_MANAGER_FD ${SOURCES_MANAGER_FD} src/manager_fd.cpp)

add_library(manager_fd STATIC ${SOURCES_MANAGER_FD})

C3QO_TARGET_COMPILE_FLAGS(manager_fd)

target_include_directories(manager_fd PRIVATE ${BLOCK_INCLUDE_DIR})
target_include_directories(manager_fd PRIVATE ${C3QO_INCLUDE_DIR})
target_include_directories(manager_fd PRIVATE ${UTILS_INCLUDE_DIR})

target_link_libraries(manager_fd logger)


## Timer manager
set(SOURCES_MANAGER_TM)
set(SOURCES_MANAGER_TM ${SOURCES_MANAGER_TM} src/manager_tm.cpp)

add_library(manager_tm STATIC ${SOURCES_MANAGER_TM})

C3QO_TARGET_COMPILE_FLAGS(manager_tm)

target_include_directories(manager_tm PRIVATE ${BLOCK_INCLUDE_DIR})
target_include_directories(manager_tm PRIVATE ${C3QO_INCLUDE_DIR})
target_include_directories(manager_tm PRIVATE ${UTILS_INCLUDE_DIR})

target_link_libraries(manager_tm logger)

## Global manager
set(SOURCES_MANAGER)
set(SOURCES_MANAGER ${SOURCES_MANAGER} src/manager.cpp)

add_library(manager STATIC ${SOURCES_MANAGER})

C3QO_TARGET_COMPILE_FLAGS(manager)

target_link_libraries(manager manager_bk)
target_link_libraries(manager manager_fd)
target_link_libraries(manager manager_tm)


# Build c3qo main executable
set(SOURCES src/main.cpp)

add_executable(c3qo ${SOURCES})

target_compile_options(c3qo PRIVATE "${C3QO_COMPILE_FLAGS}")
C3QO_TARGET_LINK_FLAGS(c3qo)

target_link_libraries(c3qo manager)


# Build TU for manager_bk
set(SOURCES_TU_MANAGER_BK test/tu_manager_bk.cpp)

add_executable(tu_manager_bk ${SOURCES_TU_MANAGER_BK})

C3QO_TARGET_LINK_FLAGS(tu_manager_bk)

add_test(NAME tu_manager_bk COMMAND tu_manager_bk)

target_link_libraries(tu_manager_bk manager)
target_link_libraries(tu_manager_bk gtest)
target_link_libraries(tu_manager_bk gtest_main)


# Build TU for manager_bk
set(SOURCES_TU_MANAGER_FD test/tu_manager_fd.cpp)

add_executable(tu_manager_fd ${SOURCES_TU_MANAGER_FD})

C3QO_TARGET_LINK_FLAGS(tu_manager_fd)

add_test(NAME tu_manager_fd COMMAND tu_manager_fd)

target_link_libraries(tu_manager_fd manager)
target_link_libraries(tu_manager_fd gtest)
target_link_libraries(tu_manager_fd gtest_main)


# Build TU for manager_bk
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
target_link_libraries(tu_perf gtest)
target_link_libraries(tu_perf gtest_main)

