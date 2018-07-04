

cmake_minimum_required(VERSION 2.8)
project(c3q0)


# Generate a JSON file with compilation commands (used with Visual Studio Code)
set(CMAKE_EXPORT_COMPILE_COMMANDS on)


if (${C3QO_RELEASE})
    set(CMAKE_BUILD_TYPE Release)
else ()
    set(CMAKE_BUILD_TYPE Debug)
endif ()


# Static compilation
if (${C3QO_STATIC})
    add_definitions(-DC3QO_STATIC)
    set(C3QO_LIB_TYPE STATIC)
else ()
    set(C3QO_LIB_TYPE SHARED)
endif ()


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


# Function to add include directories
function (C3QO_TARGET_INCLUDE t_name)
    target_include_directories(${t_name} PUBLIC ${BLOCK_INCLUDE_DIR})
    target_include_directories(${t_name} PUBLIC ${C3QO_INCLUDE_DIR})
    target_include_directories(${t_name} PUBLIC ${UTILS_INCLUDE_DIR})
endfunction (C3QO_TARGET_INCLUDE)

# Function to add compilation flags
set(C3QO_COMPILE_FLAGS -std=c++11 -Wall -Wextra -Werror)
function (C3QO_TARGET_COMPILE_FLAGS t_name)
    target_compile_options(${t_name} PRIVATE ${C3QO_COMPILE_FLAGS})

    if (${C3QO_COVERAGE})
        target_compile_options(${t_name} PRIVATE "--coverage")
    endif ()
endfunction (C3QO_TARGET_COMPILE_FLAGS)

# Function to add link flags
function (C3QO_TARGET_LINK_FLAGS t_name)
    # Required for dlopen usage and ZeroMQ runtime
    target_link_libraries(${t_name} dl)
    target_link_libraries(${t_name} zmq)

    # Get current link flags if any
    get_target_property(link_flags ${t_name} LINK_FLAGS)
    if (${link_flags} MATCHES "NOTFOUND")
        set(link_flags "")
    endif ()

    # Add some link flags
    if (${C3QO_COVERAGE})
        set(link_flags "${link_flags} --coverage")
    endif ()

    if (${NO_AS_NEEDED})
        # If specified from cmake CLI, force linked libraries
        # to appear in DT_NEEDED ELF section
        set(link_flags "${link_flags} -Wl,--no-as-needed")
    endif ()

    if (NOT "${link_flags}" STREQUAL "")
        set_target_properties(${t_name} PROPERTIES LINK_FLAGS ${link_flags})
    endif ()
endfunction (C3QO_TARGET_LINK_FLAGS)

function (c3qo_add_library target_name target_sources)
    add_library(${target_name} ${C3QO_LIB_TYPE} ${target_sources})

    c3qo_target_compile_flags(${target_name})
    c3qo_target_link_flags(${target_name})
endfunction (c3qo_add_library)

function (c3qo_add_block target_name target_sources)
    c3qo_add_library(${target_name} "${target_sources}")

    if (${C3QO_LIB_TYPE} STREQUAL SHARED)
        target_link_libraries(${target_name} logger)
    endif ()

    target_link_libraries(${target_name} manager)
endfunction (c3qo_add_block)


add_subdirectory(c3qo)
add_subdirectory(block)
add_subdirectory(utils)

