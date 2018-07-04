

# Build zmq_pair library
set(SOURCES src/zmq_pair.cpp)

c3qo_add_block(zmq_pair "${SOURCES}")

target_link_libraries(zmq_pair socket)


# Build TU for zmq_pair
set(SOURCES_TU test/tu_zmq_pair.cpp)

add_executable(tu_zmq_pair ${SOURCES_TU})

target_compile_options(tu_zmq_pair PRIVATE "-std=c++11")
C3QO_TARGET_LINK_FLAGS(tu_zmq_pair)

add_test(NAME tu_zmq_pair COMMAND tu_zmq_pair)


target_link_libraries(tu_zmq_pair zmq_pair)
target_link_libraries(tu_zmq_pair gtest)
target_link_libraries(tu_zmq_pair gtest_main)


