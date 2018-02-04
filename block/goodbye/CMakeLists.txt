

# Build block goodbye as a static library
set(SOURCES src/goodbye.cpp)


add_library(goodbye STATIC ${SOURCES})


target_include_directories(goodbye PUBLIC ${C3QO_INCLUDE_DIR})
target_include_directories(goodbye PUBLIC ${UTILS_INCLUDE_DIR})

