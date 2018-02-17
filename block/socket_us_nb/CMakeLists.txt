

# Build server_us_nb as a static library
set(SOURCES_SERVER src/server_us_nb.cpp)

add_library(server_us_nb STATIC ${SOURCES_SERVER})

target_link_libraries(server_us_nb manager)


# Build client_us_nb as a static library
set(SOURCES_CLIENT src/client_us_nb.cpp)

add_library(client_us_nb STATIC ${SOURCES_CLIENT})

target_link_libraries(client_us_nb manager)


# Build TU for socket_us_nb
set(SOURCES_TU_SOCKET_US_NB test/tu_socket_us_nb.cpp)

add_executable(tu_socket_us_nb ${SOURCES_TU_SOCKET_US_NB})

add_test(NAME tu_socket_us_nb COMMAND tu_socket_us_nb)

target_link_libraries(tu_socket_us_nb client_us_nb)
target_link_libraries(tu_socket_us_nb server_us_nb)
target_link_libraries(tu_socket_us_nb gtest)
target_link_libraries(tu_socket_us_nb gtest_main)

