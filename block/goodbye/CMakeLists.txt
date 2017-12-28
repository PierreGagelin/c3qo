set(SOURCES src/goodbye.c)


add_library(goodbye STATIC ${SOURCES})


target_include_directories(goodbye PUBLIC ${C3QO_BLOCK_INCLUDE_DIR})
target_include_directories(goodbye PUBLIC ${C3QO_LOGGER_INCLUDE_DIR})


