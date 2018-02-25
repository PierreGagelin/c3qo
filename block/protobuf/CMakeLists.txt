

include(FindProtobuf)
find_package(Protobuf REQUIRED)

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS src/test.proto)

add_library(pb_person ${PROTO_SRCS} ${PROTO_HDRS})

target_include_directories(pb_person PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
target_include_directories(pb_person PUBLIC ${Protobuf_INCLUDE_DIRS})

target_link_libraries(pb_person ${Protobuf_LIBRARIES})

add_executable(pb_test src/main.cpp)
target_link_libraries(pb_test pb_person)
