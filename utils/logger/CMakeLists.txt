

# Build logger as a static library
set(SOURCES src/logger.cpp)

add_library(logger STATIC ${SOURCES})

C3QO_TARGET_COMPILE_FLAGS(logger)

target_include_directories(logger PUBLIC ${UTILS_INCLUDE_DIR})


# Build TU for logger
set(SOURCES_TU_LOGGER test/tu_logger.cpp)

add_executable(tu_logger ${SOURCES_TU_LOGGER})

C3QO_TARGET_LINK_FLAGS(tu_logger)

add_test(NAME tu_logger COMMAND tu_logger)

target_link_libraries(tu_logger logger)
target_link_libraries(tu_logger gtest)
target_link_libraries(tu_logger gtest_main)

