set(SOURCES src/goodbye.c)

add_library(goodbye MODULE ${SOURCES})

target_include_directories(goodbye PUBLIC ${C3QO_BLOCK_INCLUDE_DIR})
