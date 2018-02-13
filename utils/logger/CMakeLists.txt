

# Build logger as a static library
set(SOURCES src/logger.cpp)

add_library(logger STATIC ${SOURCES})

target_include_directories(logger PUBLIC ${UTILS_INCLUDE_DIR})


# Build TU for logger
set(SOURCES_TU_LOGGER test/tu_logger.cpp)

add_executable(tu_logger ${SOURCES_TU_LOGGER})
add_test(NAME tu_logger COMMAND tu_logger)

target_include_directories(tu_logger PUBLIC ${UTILS_INCLUDE_DIR})

target_link_libraries(tu_logger logger)
target_link_libraries(tu_logger gtest)
target_link_libraries(tu_logger gtest_main)

