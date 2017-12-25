

# Build socket as a static library
set(SOURCES src/socket.cpp)

add_library(socket STATIC ${SOURCES})

target_include_directories(socket PUBLIC ${UTILS_INCLUDE_DIR})
target_include_directories(socket PUBLIC ${UTILS_INCLUDE_DIR})

