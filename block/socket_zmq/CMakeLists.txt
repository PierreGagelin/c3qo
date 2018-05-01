

# Build zmq_pair as a static library
set(SOURCES_ZMQ_PAIR src/zmq_pair.cpp)

add_library(zmq_pair STATIC ${SOURCES_ZMQ_PAIR})

C3QO_TARGET_COMPILE_FLAGS(zmq_pair)

target_link_libraries(zmq_pair manager_bk)


# Build TU for zmq_pair
set(SOURCES_TU test/tu_zmq_pair.cpp)

add_executable(tu_zmq_pair ${SOURCES_TU})

C3QO_TARGET_LINK_FLAGS(tu_zmq_pair)

add_test(NAME tu_zmq_pair COMMAND tu_zmq_pair)


target_link_libraries(tu_zmq_pair zmq_pair)
target_link_libraries(tu_zmq_pair gtest)
target_link_libraries(tu_zmq_pair gtest_main)


