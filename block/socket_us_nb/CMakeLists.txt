

# Build socket_us_nb library
set(SOURCES)
set(SOURCES ${SOURCES} src/client_us_nb.cpp)
set(SOURCES ${SOURCES} src/server_us_nb.cpp)

c3qo_add_block(socket_us_nb "${SOURCES}")
target_link_libraries(socket_us_nb socket)


if (NOT ${GTEST_DISABLE})
    # Build TU for socket_us_nb
    c3qo_add_test(tu_socket_us_nb test/tu_socket_us_nb.cpp)
    target_link_libraries(tu_socket_us_nb socket_us_nb)
endif ()

