

cmake_minimum_required(VERSION 2.8)
project(c3qo)


# Generate a JSON file with compilation commands (used with Visual Studio Code)
set(CMAKE_EXPORT_COMPILE_COMMANDS on)


if (${C3QO_RELEASE})
    set(CMAKE_BUILD_TYPE Release)
else ()
    set(CMAKE_BUILD_TYPE Debug)
endif ()

set(CMAKE_CXX_FLAGS_RELEASE -O2)

# Using GCOV on the code
if (${C3QO_COVERAGE})
    # Force the build type to Debug
    set(CMAKE_BUILD_TYPE Debug)
endif ()


#
# Disable log:
#   - if asked
#   - if we are making code coverage
#   - if we are in release
#
if (${LOGGER_DISABLE} OR ${C3QO_COVERAGE} OR (${CMAKE_BUILD_TYPE} STREQUAL Release))
    add_definitions(-DLOGGER_DISABLE)
endif ()


# Configure gtest as an external project
if (NOT ${GTEST_DISABLE})
    configure_file(CMakeLists.txt.in googletest-download/CMakeLists.txt)
    execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" . WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/googletest-download)
    execute_process(COMMAND ${CMAKE_COMMAND} --build                 . WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/googletest-download)

    # Add gtest directly to our build. This defines the gtest and gtest_main targets
    add_subdirectory(${CMAKE_BINARY_DIR}/googletest-src ${CMAKE_BINARY_DIR}/googletest-src EXCLUDE_FROM_ALL)
endif ()

# Configure memory checking
find_program(MEMORYCHECK_COMMAND valgrind)
set(MEMORYCHECK_COMMAND_OPTIONS "--leak-check=full --show-leak-kinds=all")
set(MEMORYCHECK_SUPPRESSIONS_FILE ${CMAKE_SOURCE_DIR}/c3qo_valgrind.supp)

# Allow tests to register themselves as tests
include(CTest)

# Library to define libraries, executables, and tests according to the project
include(c3qo_lib.cmake)

add_subdirectory(c3qo)
add_subdirectory(block)
add_subdirectory(utils)
add_subdirectory(integration)


# CPack configuration
set(CPACK_GENERATOR TGZ)
set(CPACK_PACKAGE_VERSION_MAJOR 0)
set(CPACK_PACKAGE_VERSION_MINOR 0)
set(CPACK_PACKAGE_VERSION_PATCH 7)
set(CPACK_SYSTEM_NAME local)
include(CPack)
