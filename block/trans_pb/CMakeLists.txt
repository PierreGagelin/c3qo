

# Generate configuration interface
c3qo_add_library_protobuf_c(pb_config src conf)

# Add block
c3qo_add_block(trans_pb src/trans_pb.cpp)
target_include_directories(trans_pb PUBLIC include/)
target_link_libraries(trans_pb pb_config)
target_link_libraries(trans_pb buffer)

# Build test unit
if (${C3QO_TEST})
    c3qo_add_test(tu_trans_pb test/tu_trans_pb.cpp)
    target_link_libraries(tu_trans_pb trans_pb)
endif()
