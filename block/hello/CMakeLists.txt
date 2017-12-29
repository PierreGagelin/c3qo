# Build block hello as a static library
set(SOURCES src/hello.c)


add_library(hello STATIC ${SOURCES})


target_include_directories(hello PUBLIC ${C3QO_BLOCK_INCLUDE_DIR})
target_include_directories(hello PUBLIC ${C3QO_LOGGER_INCLUDE_DIR})


# Build test tu_hello as an executable
set(SOURCES_TEST test/tu_hello.cpp)

add_executable(tu_hello ${SOURCES_TEST})

target_include_directories(tu_hello PUBLIC "${gtest_SOURCE_DIR}/include")
target_include_directories(tu_hello PUBLIC ${C3QO_LOGGER_INCLUDE_DIR})

target_link_libraries(tu_hello hello)
target_link_libraries(tu_hello gtest)
target_link_libraries(tu_hello gtest_main)
