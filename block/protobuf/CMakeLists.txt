

# Add the protobuf package
include(FindProtobuf)
find_package(Protobuf REQUIRED)
set(PROTOBUF_USE_STATIC_LIBS on)


# Build protobuf person library
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS src/test.proto)
set(SOURCES ${PROTO_SRCS} ${PROTO_HDRS})

add_library(pb_person ${SOURCES})

C3QO_TARGET_COMPILE_FLAGS(pb_person)

target_include_directories(pb_person PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
target_include_directories(pb_person PUBLIC ${PROTOBUF_INCLUDE_DIRS})

target_link_libraries(pb_person ${PROTOBUF_LITE_LIBRARIES})
#target_link_libraries(pb_person ${PROTOBUF_LIBRARIES})


# Build test tu_pb_person as an executable
set(SOURCES_TEST src/tu_pb_person.cpp)

add_executable(tu_pb_person ${SOURCES_TEST})

C3QO_TARGET_LINK_FLAGS(tu_pb_person)

target_include_directories(tu_pb_person PUBLIC ${UTILS_INCLUDE_DIR})

add_test(NAME tu_pb_person COMMAND tu_pb_person)

target_link_libraries(tu_pb_person pb_person)
target_link_libraries(tu_pb_person logger)
target_link_libraries(tu_pb_person gtest)
target_link_libraries(tu_pb_person gtest_main)
