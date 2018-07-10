

# Build logger library
c3qo_add_library(logger src/logger.cpp)


if (NOT ${GTEST_DISABLE})
    # Build TU for logger
    c3qo_add_test(tu_logger test/tu_logger.cpp)
    target_link_libraries(tu_logger logger)
endif ()
