cmake_minimum_required(VERSION 3.0)
project(c3q0)

set(CMAKE_BUILD_TYPE Debug)


# engine include directories
set(C3QO_BLOCK_INCLUDE_DIR  ${CMAKE_SOURCE_DIR}/c3qo/block/include)
set(C3QO_SIGNAL_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/c3qo/signal/include)
set(C3QO_SOCKET_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/c3qo/socket/include)


# blocks include directories
set(BLOCK_HELLO_INCLUDE_DIR          ${CMAKE_SOURCE_DIR}/block/hello/include)
set(BLOCK_GOODBYE_INCLUDE_DIR        ${CMAKE_SOURCE_DIR}/block/goodbye/include)
set(BLOCK_INOTIFY_SB_INCLUDE_DIR     ${CMAKE_SOURCE_DIR}/block/inotify_sb/include)
set(BLOCK_CLIENT_US_ASNB_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/block/client_us_asnb/include)
set(BLOCK_SERVER_US_ASNB_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/block/server_us_asnb/include)


add_subdirectory(c3qo)
add_subdirectory(block)

add_subdirectory(gtest)
