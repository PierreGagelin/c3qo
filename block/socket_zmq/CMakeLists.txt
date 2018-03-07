

# Build server_zmq_rr as a static library
set(SOURCES_SERVER src/server_zmq_rr.cpp)

add_library(server_zmq_rr STATIC ${SOURCES_SERVER})

C3QO_TARGET_COMPILE_FLAGS(server_zmq_rr)

target_link_libraries(server_zmq_rr manager_bk)


# Build client_zmq_rr as a static library
set(SOURCES_CLIENT src/client_zmq_rr.cpp)

add_library(client_zmq_rr STATIC ${SOURCES_CLIENT})

C3QO_TARGET_COMPILE_FLAGS(client_zmq_rr)

target_link_libraries(client_zmq_rr manager_bk)


# Build TU for socket_zmq_rr
set(SOURCES_TU test/tu_socket_zmq_rr.cpp)

add_executable(tu_socket_zmq_rr ${SOURCES_TU})

C3QO_TARGET_LINK_FLAGS(tu_socket_zmq_rr)

add_test(NAME tu_socket_zmq_rr COMMAND tu_socket_zmq_rr)



target_link_libraries(tu_socket_zmq_rr client_zmq_rr)
target_link_libraries(tu_socket_zmq_rr server_zmq_rr)
target_link_libraries(tu_socket_zmq_rr gtest)
target_link_libraries(tu_socket_zmq_rr gtest_main)


# Build mains to test
set(SOURCES_TEST_SERVER test/main_server.cpp)
add_executable(test_server_zmq_rr ${SOURCES_TEST_SERVER})
C3QO_TARGET_LINK_FLAGS(test_server_zmq_rr)

target_link_libraries(test_server_zmq_rr server_zmq_rr)

set(SOURCES_TEST_CLIENT test/main_client.cpp)
add_executable(test_client_zmq_rr ${SOURCES_TEST_CLIENT})
C3QO_TARGET_LINK_FLAGS(test_client_zmq_rr)

target_link_libraries(test_client_zmq_rr client_zmq_rr)
