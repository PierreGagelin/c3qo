

# Build socket library
set(SOURCES src/socket.cpp)

c3qo_add_library(socket "${SOURCES}")

target_link_libraries(socket logger)

target_include_directories(socket PUBLIC ${UTILS_INCLUDE_DIR})

