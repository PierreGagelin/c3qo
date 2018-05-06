

# Build socket library
set(SOURCES src/socket.cpp)

add_library(socket SHARED ${SOURCES})

C3QO_TARGET_COMPILE_FLAGS(socket)
C3QO_TARGET_LINK_FLAGS(socket)

target_link_libraries(socket logger)

target_include_directories(socket PUBLIC ${UTILS_INCLUDE_DIR})

