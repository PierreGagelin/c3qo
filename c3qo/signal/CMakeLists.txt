set(SOURCES src/signal.c)


add_library(signal STATIC ${SOURCES})


target_include_directories(signal PUBLIC ${C3QO_SIGNAL_INCLUDE_DIR})
target_include_directories(signal PUBLIC ${C3QO_LOGGER_INCLUDE_DIR})


