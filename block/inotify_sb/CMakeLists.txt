set(SOURCES src/inotify_sb.c)


add_library(inotify_sb STATIC ${SOURCES})


target_include_directories(inotify_sb PUBLIC ${C3QO_BLOCK_INCLUDE_DIR})
target_include_directories(inotify_sb PUBLIC ${C3QO_LOGGER_INCLUDE_DIR})


