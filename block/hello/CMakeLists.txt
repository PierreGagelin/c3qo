

# Build block hello library
c3qo_add_block(hello src/hello.cpp)
target_include_directories(hello PUBLIC include/)


if (${C3QO_TEST})
    # Build test unit
    c3qo_add_test(tu_hello test/tu_hello.cpp)
    target_link_libraries(tu_hello hello)
endif()
