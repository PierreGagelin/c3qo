

# Build block hello as a static library
set(SOURCES src/hello.cpp)

add_library(hello STATIC ${SOURCES})

target_link_libraries(hello manager)


# Build test tu_hello as an executable
set(SOURCES_TEST test/tu_hello.cpp)

add_executable(tu_hello ${SOURCES_TEST})

add_test(NAME tu_hello COMMAND tu_hello)

target_link_libraries(tu_hello hello)
target_link_libraries(tu_hello gtest)
target_link_libraries(tu_hello gtest_main)

