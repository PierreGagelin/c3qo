

# Build block hello library
set(SOURCES src/hello.cpp)

c3qo_add_block(hello "${SOURCES}")


# Build test tu_hello as an executable
set(SOURCES_TEST test/tu_hello.cpp)

add_executable(tu_hello ${SOURCES_TEST})

target_compile_options(tu_hello PRIVATE "-std=c++11")
C3QO_TARGET_LINK_FLAGS(tu_hello)

add_test(NAME tu_hello COMMAND tu_hello)

target_link_libraries(tu_hello hello)
target_link_libraries(tu_hello gtest)
target_link_libraries(tu_hello gtest_main)

