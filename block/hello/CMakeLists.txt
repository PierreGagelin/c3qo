

# Build block hello library
c3qo_add_block(hello src/hello.cpp)


if (NOT ${GTEST_DISABLE})
    # Build test unit
    c3qo_add_test(tu_hello test/tu_hello.cpp)
    target_link_libraries(tu_hello hello)
endif ()

