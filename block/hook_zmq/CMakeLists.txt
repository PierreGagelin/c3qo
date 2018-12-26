

# Build hook_zmq library
c3qo_add_block(hook_zmq src/hook_zmq.cpp)
target_include_directories(hook_zmq PUBLIC include/)
target_link_libraries(hook_zmq hello)
target_link_libraries(hook_zmq trans_pb)
target_link_libraries(hook_zmq buffer)


if (${C3QO_TEST})
    # Build TU for hook_zmq
    c3qo_add_test(tu_hook_zmq test/tu_hook_zmq.cpp)
    target_link_libraries(tu_hook_zmq hook_zmq)
endif()
