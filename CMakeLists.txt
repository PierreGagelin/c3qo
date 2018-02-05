

cmake_minimum_required(VERSION 3.0)
project(c3q0)

# Generate a JSON file with compilation commands (used with Visual Studio Code)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Build type: either Debug or Release
set(CMAKE_BUILD_TYPE Debug)

# Compilation flags
set(CMAKE_CXX_FLAGS_DEBUG   "-g -O0")
set(CMAKE_CXX_FLAGS_RELEASE "-O2")
set(CMAKE_CXX_FLAGS         "-std=c++11 -Wall -Wextra -Werror")

# Add a flag to disable logs on Release
IF (${CMAKE_BUILD_TYPE} STREQUAL Release)
    add_definitions(-DLOGGER_DISABLE)
ENDIF()

# Generate tests executables
enable_testing()

# Include directories for c3qo engine
set(C3QO_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/c3qo/include)

# Include directories for utils
set(UTILS_INCLUDE_DIR)
set(UTILS_INCLUDE_DIR ${UTILS_INCLUDE_DIR} ${CMAKE_SOURCE_DIR}/utils/logger/include)
set(UTILS_INCLUDE_DIR ${UTILS_INCLUDE_DIR} ${CMAKE_SOURCE_DIR}/utils/socket/include)

add_subdirectory(c3qo)
add_subdirectory(block)
add_subdirectory(utils)
add_subdirectory(gtest)

