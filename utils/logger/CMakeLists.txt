

# Build logger library
c3qo_add_library(logger src/logger.cpp)
target_include_directories(logger PUBLIC include/)


if (${C3QO_TEST})
    # Build TU for logger
    c3qo_add_test(tu_logger test/tu_logger.cpp)
    target_link_libraries(tu_logger logger)
endif()
