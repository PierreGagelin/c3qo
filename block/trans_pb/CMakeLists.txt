

# Add block
c3qo_add_block(trans_pb src/trans_pb.cpp)

if (${C3QO_PROTOBUF})
    # Specify that protobuf is used to the block
    target_compile_options(trans_pb PUBLIC -DC3QO_PROTOBUF)
    #add_definitions(-DC3QO_PROTOBUF)

    # Generate protobuf library
    c3qo_add_library_protobuf(block_pb src/block.proto)

    # Link the protobuf library to the block
    target_link_libraries(trans_pb block_pb)

    # Build test unit
    if (NOT ${GTEST_DISABLE})
        c3qo_add_test(tu_trans_pb test/tu_trans_pb.cpp)
        target_link_libraries(tu_trans_pb trans_pb)
    endif ()
endif ()
