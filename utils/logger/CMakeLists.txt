

# Build logger library
set(SOURCES src/logger.cpp)

add_library(logger SHARED ${SOURCES})

c3qo_target_compile_flags(logger)
c3qo_target_link_flags(logger)

target_include_directories(logger PUBLIC ${UTILS_INCLUDE_DIR})


# Build TU for logger
set(SOURCES_TU test/tu_logger.cpp)

c3qo_add_test(tu_logger "${SOURCES_TU}")

target_link_libraries(tu_logger logger)

