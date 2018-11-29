

# Generate protobuf library
c3qo_add_library_protobuf_c(block_pb src block)

# Add block
c3qo_add_block(trans_pb src/trans_pb.cpp)
target_include_directories(trans_pb PUBLIC include/)
target_link_libraries(trans_pb hello)
target_link_libraries(trans_pb zmq_pair)
target_link_libraries(trans_pb block_pb)
target_link_libraries(trans_pb socket)

# Build test unit
if (${C3QO_TEST})
    c3qo_add_test(tu_trans_pb test/tu_trans_pb.cpp)
    c3qo_link_block(tu_trans_pb trans_pb)
endif()
