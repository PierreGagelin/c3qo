

# Build block hello as a static library
set(SOURCES src/hello.cpp)

add_library(hello STATIC ${SOURCES})

target_include_directories(hello PUBLIC ${BLOCK_INCLUDE_DIR})
target_include_directories(hello PUBLIC ${C3QO_INCLUDE_DIR})
target_include_directories(hello PUBLIC ${UTILS_INCLUDE_DIR})


# Build test tu_hello as an executable
set(SOURCES_TEST test/tu_hello.cpp)

add_executable(tu_hello ${SOURCES_TEST})
add_test(NAME tu_hello COMMAND tu_hello)

target_include_directories(tu_hello PUBLIC ${BLOCK_INCLUDE_DIR})
target_include_directories(tu_hello PUBLIC ${C3QO_INCLUDE_DIR})
target_include_directories(tu_hello PUBLIC ${UTILS_INCLUDE_DIR})

target_link_libraries(tu_hello hello)
target_link_libraries(tu_hello manager)
target_link_libraries(tu_hello logger)
target_link_libraries(tu_hello gtest)
target_link_libraries(tu_hello gtest_main)

