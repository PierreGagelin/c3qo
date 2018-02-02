

set(SOURCES src/logger.cpp)


add_library(logger STATIC ${SOURCES})


target_include_directories(logger PUBLIC ${C3QO_LOGGER_INCLUDE_DIR})


