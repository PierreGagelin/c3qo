

cmake_minimum_required(VERSION 3.0)
project(c3q0)


set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


# Compile with DEBUG level
set(CMAKE_BUILD_TYPE Debug)


# Compilation flags
set(CMAKE_C_FLAGS_DEBUG   "-g -O0")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")

set(CMAKE_C_FLAGS_RELEASE   "-O2")
set(CMAKE_CXX_FLAGS_RELEASE "-O2")

set(CMAKE_C_FLAGS   "-Wall -Wextra -Werror")
set(CMAKE_CXX_FLAGS "-std=c++11 -Wall -Wextra -Werror")


# Allow to run "make test"
enable_testing()


# Engine include directories
set(C3QO_BLOCK_INCLUDE_DIR   ${CMAKE_SOURCE_DIR}/c3qo/block/include)
set(C3QO_LOGGER_INCLUDE_DIR  ${CMAKE_SOURCE_DIR}/c3qo/logger/include)
set(C3QO_MANAGER_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/c3qo/engine/include)
set(C3QO_SOCKET_INCLUDE_DIR  ${CMAKE_SOURCE_DIR}/c3qo/socket/include)


# Utils include directory
set(UTILS_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/utils/include)


add_subdirectory(c3qo)
add_subdirectory(block)
add_subdirectory(utils)

add_subdirectory(gtest)


