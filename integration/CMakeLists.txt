

include(FindProtobuf)
find_package(Protobuf REQUIRED)

configure_file(tools/c3qo_cli_pb.py tools/c3qo_cli_pb.py COPYONLY)
configure_file(tools/server_zmq_pb.txt tools/server_zmq_pb.txt COPYONLY)
install(FILES tools/c3qo_cli_pb.py DESTINATION test)
install(FILES tools/server_zmq_pb.txt DESTINATION test)
install(FILES server_zmq.txt DESTINATION test)
install(FILES server_us_nb.txt DESTINATION test)

protobuf_generate_python(PROTO_SRCS_PY ../block/trans_pb/src/block.proto)
add_custom_target(block_pb_py ALL
                  COMMAND cp ${PROTO_SRCS_PY} ${CMAKE_CURRENT_BINARY_DIR}/tools/
                  DEPENDS ${PROTO_SRCS_PY})
install(FILES ${PROTO_SRCS_PY} DESTINATION test)

protobuf_generate_python(PROTO_SRCS_PY ../c3qo/src/conf.proto)
add_custom_target(conf_pb_py ALL
                  COMMAND cp ${PROTO_SRCS_PY} ${CMAKE_CURRENT_BINARY_DIR}/tools/
                  DEPENDS ${PROTO_SRCS_PY})
install(FILES ${PROTO_SRCS_PY} DESTINATION test)
