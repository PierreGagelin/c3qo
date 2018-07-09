
# Nothing to build if protobuf is disabled
if (NOT ${C3QO_PROTOBUF})
    return()
endif ()


# Generate protobuf library
c3qo_add_library_protobuf(block_pb src/block.proto)


# Add block
c3qo_add_block(trans_pb src/trans_pb.cpp)
target_link_libraries(trans_pb block_pb)


# Build test unit
c3qo_add_test(tu_trans_pb test/tu_trans_pb.cpp)
target_link_libraries(tu_trans_pb trans_pb)
target_link_libraries(tu_trans_pb block_pb)
