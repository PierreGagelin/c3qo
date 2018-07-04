

# Build zmq_pair library
set(SOURCES src/zmq_pair.cpp)

c3qo_add_block(zmq_pair "${SOURCES}")

target_link_libraries(zmq_pair socket)


# Build TU for zmq_pair
set(SOURCES_TU test/tu_zmq_pair.cpp)

c3qo_add_test(tu_zmq_pair "${SOURCES_TU}")

target_link_libraries(tu_zmq_pair zmq_pair)

