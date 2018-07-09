

# Build zmq_pair library
c3qo_add_block(zmq_pair src/zmq_pair.cpp)
target_link_libraries(zmq_pair socket)


if (NOT ${GTEST_DISABLE})
    # Build TU for zmq_pair
    c3qo_add_test(tu_zmq_pair test/tu_zmq_pair.cpp)
    target_link_libraries(tu_zmq_pair zmq_pair)
endif ()

