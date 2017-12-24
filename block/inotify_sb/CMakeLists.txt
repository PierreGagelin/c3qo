cmake_minimum_required(VERSION 3.0)
project(inotify_sb)

set(SOURCES src/inotify_sb.c)

add_library(inotify_sb MODULE ${SOURCES})

target_include_directories(inotify_sb PUBLIC ${C3QO_BLOCK_INCLUDE_DIR})
