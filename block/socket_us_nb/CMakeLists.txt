

# Build socket_us_nb library
set(SOURCES)
set(SOURCES ${SOURCES} src/client_us_nb.cpp)
set(SOURCES ${SOURCES} src/server_us_nb.cpp)

add_library(socket_us_nb ${C3QO_LIB_TYPE} ${SOURCES})

C3QO_TARGET_COMPILE_FLAGS(socket_us_nb)
C3QO_TARGET_LINK_FLAGS(socket_us_nb)

target_link_libraries(socket_us_nb manager)
target_link_libraries(socket_us_nb socket)


# Build TU for socket_us_nb
set(SOURCES_TU_SOCKET_US_NB test/tu_socket_us_nb.cpp)

add_executable(tu_socket_us_nb ${SOURCES_TU_SOCKET_US_NB})

target_compile_options(tu_socket_us_nb PRIVATE "-std=c++11")
C3QO_TARGET_LINK_FLAGS(tu_socket_us_nb)

add_test(NAME tu_socket_us_nb COMMAND tu_socket_us_nb)

target_link_libraries(tu_socket_us_nb socket_us_nb)
target_link_libraries(tu_socket_us_nb gtest)
target_link_libraries(tu_socket_us_nb gtest_main)

