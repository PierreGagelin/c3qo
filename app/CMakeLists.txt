

# Build c3qo main executable
c3qo_add_executable(c3qo src/main.cpp)
target_link_libraries(c3qo manager)
target_link_libraries(c3qo hello)
target_link_libraries(c3qo trans_pb)
target_link_libraries(c3qo hook_zmq)

# Build network CLI executable
c3qo_add_executable(ncli src/network_cli.cpp)
target_link_libraries(ncli pb_config)
target_link_libraries(ncli socket)
