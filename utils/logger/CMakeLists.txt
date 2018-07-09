

# Build logger library
add_library(logger SHARED src/logger.cpp)

c3qo_target_compile_flags(logger)
c3qo_target_link_flags(logger)

target_include_directories(logger PUBLIC ${UTILS_INCLUDE_DIR})


if (NOT ${GTEST_DISABLE})
    # Build TU for logger
    c3qo_add_test(tu_logger test/tu_logger.cpp)
    target_link_libraries(tu_logger logger)
endif ()
