cmake_minimum_required(VERSION 3.0)
project(hello)

set(SOURCES common/src/hello.c)

add_library(hello MODULE ${SOURCES})

target_include_directories(hello PUBLIC ${C3QO_BLOCK_INCLUDE_DIR})
