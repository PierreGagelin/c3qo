

# Build zmq_pair library
c3qo_add_block(zmq_pair src/zmq_pair.cpp)
target_include_directories(zmq_pair PUBLIC include/)
target_link_libraries(zmq_pair hello)
target_link_libraries(zmq_pair trans_pb)
target_link_libraries(zmq_pair socket)


if (${C3QO_TEST})
    # Build TU for zmq_pair
    c3qo_add_test(tu_zmq_pair test/tu_zmq_pair.cpp)
    target_link_libraries(tu_zmq_pair zmq_pair)
endif()
