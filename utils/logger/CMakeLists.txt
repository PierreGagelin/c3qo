

# Build logger as a static library
set(SOURCES src/logger.cpp)


add_library(logger STATIC ${SOURCES})


target_include_directories(logger PUBLIC ${UTILS_INCLUDE_DIR})


