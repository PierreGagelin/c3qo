#
# C3QO build system
#

cmake_minimum_required(VERSION 2.8)
project(c3qo)

# Generate a JSON file with compilation commands (used with Visual Studio Code)
set(CMAKE_EXPORT_COMPILE_COMMANDS on)

# If not empty an undesired "rdynamic" option is added
set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS "")

set(CMAKE_INSTALL_PREFIX /tmp/c3qo)
set(CMAKE_INSTALL_RPATH $ORIGIN/../lib)

#
# GCOV
#
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

#
# CTest configuration
#
find_program(MEMORYCHECK_COMMAND valgrind)
set(MEMORYCHECK_COMMAND_OPTIONS "--leak-check=full --show-leak-kinds=all")
include(CTest)

#
# CPack configuration
#
set(CPACK_GENERATOR TGZ)
set(CPACK_PACKAGE_VERSION_MAJOR 0)
set(CPACK_PACKAGE_VERSION_MINOR 0)
set(CPACK_PACKAGE_VERSION_PATCH 7)
set(CPACK_SYSTEM_NAME local)
include(CPack)

# Library to define libraries, executables, and tests according to the project
include(c3qo_lib.cmake)

add_subdirectory(app)
add_subdirectory(block)
add_subdirectory(engine)
add_subdirectory(utils)
