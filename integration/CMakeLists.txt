

include(FindProtobuf)
find_package(Protobuf REQUIRED)

configure_file(tools/c3qo_cli_pb.py tools/ COPYONLY)
configure_file(tools/server_zmq_pb.txt tools/ COPYONLY)
install(FILES tools/c3qo_cli_pb.py DESTINATION test)
install(FILES tools/server_zmq_pb.txt DESTINATION test)

protobuf_generate_python(PROTO_SRCS_PY ../block/trans_pb/src/block.proto)
add_custom_target(toto_pb_py ALL DEPENDS ${PROTO_SRCS_PY})
configure_file(${PROTO_SRCS_PY} tools/ COPYONLY)
install(FILES ${PROTO_SRCS_PY} DESTINATION test)
