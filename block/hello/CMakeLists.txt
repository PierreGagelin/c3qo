

# Build block hello library
set(SOURCES src/hello.cpp)

c3qo_add_block(hello "${SOURCES}")


# Build test tu_hello as an executable
set(SOURCES_TU test/tu_hello.cpp)

c3qo_add_test(tu_hello ${SOURCES_TU})

target_link_libraries(tu_hello hello)

