#
# C3QO build system
#

cmake_minimum_required(VERSION 2.8)
project(c3qo)

# Generate a JSON file with compilation commands (used with Visual Studio Code)
set(CMAKE_EXPORT_COMPILE_COMMANDS on)

# Force a level of optimization
set(CMAKE_CXX_FLAGS_RELEASE -O2)
set(CMAKE_C_FLAGS_RELEASE -O2)

# Using GCOV on the code
if (${C3QO_COVERAGE})
    # Force the build type to Debug
    set(CMAKE_BUILD_TYPE Debug)
endif()

#
# Enable logger
#
if (${C3QO_LOG})
    add_definitions(-DC3QO_LOG)
endif()

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


# CPack configuration
set(CPACK_GENERATOR TGZ)
set(CPACK_PACKAGE_VERSION_MAJOR 0)
set(CPACK_PACKAGE_VERSION_MINOR 0)
set(CPACK_PACKAGE_VERSION_PATCH 7)
set(CPACK_SYSTEM_NAME local)
include(CPack)
