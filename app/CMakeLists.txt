

# Build c3qo main executable
c3qo_add_executable(c3qo src/c3qo.cpp)
target_link_libraries(c3qo manager)
target_link_libraries(c3qo hello)
target_link_libraries(c3qo trans_pb)
target_link_libraries(c3qo hook_zmq)

# ZMQ proxy
c3qo_add_executable(proxy src/proxy.cpp)
target_link_libraries(proxy manager)
target_link_libraries(proxy hook_zmq)

# Build network CLI executable
c3qo_add_executable(ncli src/ncli.cpp)
target_link_libraries(ncli manager)
target_link_libraries(ncli pb_config)
