cmake_minimum_required(VERSION 3.0)
project(goodbye)

set(SOURCES common/src/goodbye.c)

add_library(goodbye MODULE ${SOURCES})

target_include_directories(goodbye PUBLIC ${C3QO_BLOCK_INCLUDE_DIR})
