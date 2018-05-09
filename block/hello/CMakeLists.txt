

# Build block hello library
set(SOURCES src/hello.cpp)

add_library(hello ${C3QO_LIB_TYPE} ${SOURCES})

C3QO_TARGET_COMPILE_FLAGS(hello)
C3QO_TARGET_LINK_FLAGS(hello)

target_link_libraries(hello manager)


# Build test tu_hello as an executable
set(SOURCES_TEST test/tu_hello.cpp)

add_executable(tu_hello ${SOURCES_TEST})

C3QO_TARGET_LINK_FLAGS(tu_hello)

add_test(NAME tu_hello COMMAND tu_hello)

target_link_libraries(tu_hello hello)
target_link_libraries(tu_hello gtest)
target_link_libraries(tu_hello gtest_main)

