cmake_minimum_required(VERSION 3.0)
project(c3q0)

set(CMAKE_BUILD_TYPE Debug)


# engine include directories
set(C3QO_BLOCK_INCLUDE_DIR   ${CMAKE_SOURCE_DIR}/c3qo/block/include)
set(C3QO_LOGGER_INCLUDE_DIR  ${CMAKE_SOURCE_DIR}/c3qo/logger/include)
set(C3QO_MANAGER_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/c3qo/engine/include)
set(C3QO_SIGNAL_INCLUDE_DIR  ${CMAKE_SOURCE_DIR}/c3qo/signal/include)
set(C3QO_SOCKET_INCLUDE_DIR  ${CMAKE_SOURCE_DIR}/c3qo/socket/include)


add_subdirectory(c3qo)
add_subdirectory(block)

add_subdirectory(gtest)