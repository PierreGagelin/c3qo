

# Include directories for blocks
set(BLOCK_INCLUDE_DIR)
set(BLOCK_INCLUDE_DIR ${BLOCK_INCLUDE_DIR} ${CMAKE_SOURCE_DIR}/block/hello/include)
set(BLOCK_INCLUDE_DIR ${BLOCK_INCLUDE_DIR} ${CMAKE_SOURCE_DIR}/block/project_euler/include)
set(BLOCK_INCLUDE_DIR ${BLOCK_INCLUDE_DIR} ${CMAKE_SOURCE_DIR}/block/socket_us_nb/include)
set(BLOCK_INCLUDE_DIR ${BLOCK_INCLUDE_DIR} ${CMAKE_SOURCE_DIR}/block/socket_zmq/include)
set(BLOCK_INCLUDE_DIR ${BLOCK_INCLUDE_DIR} ${CMAKE_SOURCE_DIR}/block/trans_pb/include)

# Include directories for c3qo engine
set(C3QO_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/c3qo/include)

# Include directories for utils
set(UTILS_INCLUDE_DIR)
set(UTILS_INCLUDE_DIR ${UTILS_INCLUDE_DIR} ${CMAKE_SOURCE_DIR}/utils/include)
set(UTILS_INCLUDE_DIR ${UTILS_INCLUDE_DIR} ${CMAKE_SOURCE_DIR}/utils/logger/include)
set(UTILS_INCLUDE_DIR ${UTILS_INCLUDE_DIR} ${CMAKE_SOURCE_DIR}/utils/socket/include)

# Include directories for test units
set(TU_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/c3qo/test/include)

# Compilation flags
set(CMAKE_CXX_STANDARD 11)

set(COMPILE_FLAGS_COMMON)
set(COMPILE_FLAGS_COMMON ${COMPILE_FLAGS_COMMON} -Wall)
set(COMPILE_FLAGS_COMMON ${COMPILE_FLAGS_COMMON} -Wextra)
set(COMPILE_FLAGS_COMMON ${COMPILE_FLAGS_COMMON} -fno-rtti)

# XXX: apparently causes .gcno to be corrupted under ubuntu 14.04.5 toolchain
#set(COMPILE_FLAGS_COMMON ${COMPILE_FLAGS_COMMON} -flto)

set(COMPILE_FLAGS_BLOCK ${COMPILE_FLAGS_COMMON})
set(COMPILE_FLAGS_BLOCK ${COMPILE_FLAGS_BLOCK} -Werror)
set(COMPILE_FLAGS_BLOCK ${COMPILE_FLAGS_BLOCK} -Woverloaded-virtual)
set(COMPILE_FLAGS_BLOCK ${COMPILE_FLAGS_BLOCK} -Wswitch)
set(COMPILE_FLAGS_BLOCK ${COMPILE_FLAGS_BLOCK} -Wsign-conversion)
set(COMPILE_FLAGS_BLOCK ${COMPILE_FLAGS_BLOCK} -Wshadow)

#
# Add include directories
#
function (c3qo_target_include t_name)
    target_include_directories(${t_name} PRIVATE ${BLOCK_INCLUDE_DIR})
    target_include_directories(${t_name} PRIVATE ${C3QO_INCLUDE_DIR})
    target_include_directories(${t_name} PRIVATE ${UTILS_INCLUDE_DIR})
endfunction (c3qo_target_include)

#
# Add compilation flags
#
function (c3qo_target_compile_flags t_name)
    target_compile_options(${t_name} PRIVATE ${COMPILE_FLAGS_BLOCK})

    if (${C3QO_COVERAGE})
        target_compile_options(${t_name} PRIVATE "--coverage")
    endif ()
endfunction (c3qo_target_compile_flags)

#
# Add link flags
#
function (c3qo_target_link_flags t_name)
    # Required for ZeroMQ runtime
    target_link_libraries(${t_name} zmq)

    # XXX: apparently causes .gcno to be corrupted under ubuntu 14.04.5 toolchain
    #set_property(TARGET ${t_name} APPEND_STRING PROPERTY LINK_FLAGS " -flto")

    # Add coverage link flag
    if (${C3QO_COVERAGE})
        set_property(TARGET ${t_name} APPEND_STRING PROPERTY LINK_FLAGS " --coverage")
    endif ()
endfunction (c3qo_target_link_flags)

#
# Add a library
#
function (c3qo_add_library target_name target_sources)
    add_library(${target_name} STATIC ${target_sources})

    c3qo_target_include(${target_name})
    c3qo_target_compile_flags(${target_name})
    c3qo_target_link_flags(${target_name})
endfunction (c3qo_add_library)

#
# Add a block
#
function (c3qo_add_block target_name target_sources)
    c3qo_add_library(${target_name} "${target_sources}")

    target_link_libraries(${target_name} manager)
endfunction (c3qo_add_block)

#
# Link against a block
# In case it's a STATIC library, it forces to pull translation units  (for constructor and destructor)
#
function (c3qo_link_block target_name block_name)
    target_link_libraries(${target_name} -Wl,--whole-archive ${block_name} -Wl,--no-whole-archive)
endfunction (c3qo_link_block)

#
# Add an executable
#
function (c3qo_add_executable target_name target_sources)
    add_executable(${target_name} ${target_sources})

    c3qo_target_include(${target_name})
    c3qo_target_compile_flags(${target_name})
    c3qo_target_link_flags(${target_name})

    target_link_libraries(${target_name} logger)

    install(TARGETS ${target_name} DESTINATION bin)
endfunction (c3qo_add_executable)

#
# Add a test unit
#
function (c3qo_add_test target_name target_sources)
    add_executable(${target_name} ${target_sources})

    c3qo_target_include(${target_name})
    target_include_directories(${target_name} PRIVATE ${TU_INCLUDE_DIR})

    target_compile_options(${target_name} PRIVATE ${COMPILE_FLAGS_COMMON})

    c3qo_target_link_flags(${target_name})
    target_link_libraries(${target_name} logger)
    target_link_libraries(${target_name} gtest)
    target_link_libraries(${target_name} gtest_main)

    add_test(NAME ${target_name} COMMAND ${target_name})
endfunction (c3qo_add_test)

#
# Enable protobuf
#
if (${C3QO_PROTOBUF})
    include(FindProtobuf)
    find_package(Protobuf REQUIRED)
    set(PROTOBUF_USE_STATIC_LIBS on)
endif (${C3QO_PROTOBUF})

#
# Add a protobuf library
#
function (c3qo_add_library_protobuf target_name target_sources)
    if (NOT ${C3QO_PROTOBUF})
        return()
    endif ()

    # Generate python for test
    protobuf_generate_python(PROTO_PY "${target_sources}")
    add_custom_target(${target_name}_py ALL DEPENDS ${PROTO_PY})
    install(FILES ${PROTO_PY} DESTINATION test)

    # Generate C++
    protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS "${target_sources}")

    add_library(${target_name} STATIC ${PROTO_SRCS} ${PROTO_HDRS})

    target_compile_options(${target_name} PRIVATE ${COMPILE_FLAGS_COMMON})
    target_compile_options(${target_name} PUBLIC -DGOOGLE_PROTOBUF_NO_RTTI)

    target_include_directories(${target_name} PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
    target_include_directories(${target_name} PUBLIC ${PROTOBUF_INCLUDE_DIRS})

    target_link_libraries(${target_name} ${PROTOBUF_LITE_LIBRARIES})
endfunction (c3qo_add_library_protobuf)
