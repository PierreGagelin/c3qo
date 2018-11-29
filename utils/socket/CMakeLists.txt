

# Build socket library
set(SOURCES src/socket.cpp)

c3qo_add_library(socket "${SOURCES}")
target_include_directories(socket PUBLIC include/)
target_link_libraries(socket logger)
