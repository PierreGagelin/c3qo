cmake_minimum_required(VERSION 3.0)
project(c3q0)

set(CMAKE_BUILD_TYPE Debug)

set(C3QO_BLOCK_INCLUDE_DIR  ${CMAKE_SOURCE_DIR}/c3qo/block/include)
set(C3QO_SIGNAL_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/c3qo/signal/include)
set(C3QO_SOCKET_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/c3qo/socket/include)

add_subdirectory(block)
add_subdirectory(c3qo)
