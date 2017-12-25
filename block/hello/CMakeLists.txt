# Build block hello as a static library
set(SOURCES src/hello.c)


add_library(hello STATIC ${SOURCES})


target_include_directories(hello PUBLIC ${C3QO_BLOCK_INCLUDE_DIR})

target_include_directories(hello PUBLIC ${BLOCK_HELLO_INCLUDE_DIR})


# Build test tu_hello as an executable
set(SOURCES_TEST test/tu_hello.c)

add_executable(tu_hello ${SOURCES_TEST})

target_include_directories(tu_hello PUBLIC ${BLOCK_HELLO_INCLUDE_DIR})

target_link_libraries(tu_hello debug hello)
