

cmake_minimum_required(VERSION 3.0)
project(c3q0)


# Generate a JSON file with compilation commands (used with Visual Studio Code)
set(CMAKE_EXPORT_COMPILE_COMMANDS on)


# Build type: either Debug or Release
set(CMAKE_BUILD_TYPE Debug)


# Compilation flags
set(CMAKE_CXX_FLAGS         "${CMAKE_CXX_FLAGS}")
set(CMAKE_CXX_FLAGS_DEBUG   "${CMAKE_CXX_FLAGS_DEBUG} -g -O0")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2")


# Using GCOV on the code
set(C3QO_COVERAGE off)
if (${C3QO_COVERAGE})
    # Force the build type to Debug
    set(CMAKE_BUILD_TYPE Debug)
endif()


#[=[
Disable log:
  - if asked
  - if we are making code coverage
  - if we are in release
#]=]
set(LOGGER_DISABLE off)
if (${LOGGER_DISABLE} OR ${C3QO_COVERAGE} OR (${CMAKE_BUILD_TYPE} STREQUAL Release))
    add_definitions(-DLOGGER_DISABLE)
endif()


# Configure gtest as an external project
configure_file(CMakeLists.txt.in googletest-download/CMakeLists.txt)
execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" . WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/googletest-download)
execute_process(COMMAND ${CMAKE_COMMAND} --build                 . WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/googletest-download)

# Add gtest directly to our build. This defines the gtest and gtest_main targets
add_subdirectory(${CMAKE_BINARY_DIR}/googletest-src EXCLUDE_FROM_ALL)

# Allow tests to register themselves as tests
enable_testing()


# Include directories for blocks
set(BLOCK_INCLUDE_DIR)
set(BLOCK_INCLUDE_DIR ${BLOCK_INCLUDE_DIR} ${CMAKE_SOURCE_DIR}/block/hello/include)
set(BLOCK_INCLUDE_DIR ${BLOCK_INCLUDE_DIR} ${CMAKE_SOURCE_DIR}/block/project_euler/include)
set(BLOCK_INCLUDE_DIR ${BLOCK_INCLUDE_DIR} ${CMAKE_SOURCE_DIR}/block/socket_us_nb/include)
set(BLOCK_INCLUDE_DIR ${BLOCK_INCLUDE_DIR} ${CMAKE_SOURCE_DIR}/block/socket_zmq/include)

# Include directories for c3qo engine
set(C3QO_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/c3qo/include)

# Include directories for utils
set(UTILS_INCLUDE_DIR)
set(UTILS_INCLUDE_DIR ${UTILS_INCLUDE_DIR} ${CMAKE_SOURCE_DIR}/utils/logger/include)
set(UTILS_INCLUDE_DIR ${UTILS_INCLUDE_DIR} ${CMAKE_SOURCE_DIR}/utils/socket/include)


# Function to add compilation flags
set(C3QO_COMPILE_FLAGS -std=c++11 -Wall -Wextra -Werror)
function(C3QO_TARGET_COMPILE_FLAGS t_name)
    target_compile_options(${t_name} PRIVATE "${C3QO_COMPILE_FLAGS}")
    if(${C3QO_COVERAGE})
        target_compile_options(${t_name} PRIVATE "--coverage")
    endif()
endfunction(C3QO_TARGET_COMPILE_FLAGS)

# Function to add link flags
function(C3QO_TARGET_LINK_FLAGS t_name)
    set_target_properties(${t_name} PROPERTIES LINK_FLAGS "-lzmq")
    if(${C3QO_COVERAGE})    
        set_target_properties(${t_name} PROPERTIES LINK_FLAGS "--coverage")
    endif()
endfunction(C3QO_TARGET_LINK_FLAGS)


add_subdirectory(c3qo)
add_subdirectory(block)
add_subdirectory(utils)

