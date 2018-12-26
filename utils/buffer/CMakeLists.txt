

# Build buffer library
c3qo_add_library(buffer src/buffer.cpp)
target_include_directories(buffer PUBLIC include/)
target_link_libraries(buffer logger)
