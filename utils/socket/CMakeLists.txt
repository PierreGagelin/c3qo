

# Build socket as a static library
set(SOURCES src/socket.cpp)

add_library(socket STATIC ${SOURCES})

C3QO_TARGET_COMPILE_FLAGS(socket)

target_include_directories(socket PUBLIC ${UTILS_INCLUDE_DIR})

