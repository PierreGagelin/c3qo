

# Build vector as a STATIC library
set(SOURCES src/vector.cpp)


add_library(vector STATIC ${SOURCES})


target_include_directories(vector PUBLIC ${UTILS_INCLUDE_DIR})


# Build test tu_vector as an executable
set(SOURCES_TEST test/tu_vector.cpp)


add_executable(tu_vector ${SOURCES_TEST})
add_test(NAME tu_vector COMMAND tu_vector)


target_include_directories(tu_vector PUBLIC ${UTILS_INCLUDE_DIR})

target_include_directories(tu_vector PUBLIC "${gtest_SOURCE_DIR}/include")


target_link_libraries(tu_vector vector)
target_link_libraries(tu_vector gtest)
target_link_libraries(tu_vector gtest_main)


