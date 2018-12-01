

# Build c3qo main executable
c3qo_add_executable(c3qo src/main.cpp)
target_link_libraries(c3qo manager)
c3qo_link_block(c3qo hello)
c3qo_link_block(c3qo trans_pb)
c3qo_link_block(c3qo zmq_pair)

# Build network CLI executable
c3qo_add_executable(ncli src/network_cli.cpp)
target_link_libraries(ncli pb_config)
target_link_libraries(ncli socket)
