cmake_minimum_required(VERSION 3.0)
project(signal)

set(SOURCES src/signal.c)

add_library(signal MODULE ${SOURCES})

target_include_directories(signal PUBLIC ${C3QO_SIGNAL_INCLUDE_DIR})
