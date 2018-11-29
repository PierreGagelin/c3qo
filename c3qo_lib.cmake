

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

# C compilations flags
set(CMAKE_C_STANDARD 11)
set(COMPILE_FLAGS_C)
set(COMPILE_FLAGS_C ${COMPILE_FLAGS_C} -Wall)
set(COMPILE_FLAGS_C ${COMPILE_FLAGS_C} -Wextra)

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

    target_include_directories(${target_name} PUBLIC ${CMAKE_SOURCE_DIR}/utils/include/)
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

    target_include_directories(${target_name} PRIVATE ${TU_INCLUDE_DIR})

    target_compile_options(${target_name} PRIVATE ${COMPILE_FLAGS_COMMON})

    c3qo_target_link_flags(${target_name})
    target_link_libraries(${target_name} logger)

    add_test(NAME ${target_name} COMMAND ${target_name})
endfunction (c3qo_add_test)

#
# Add a protobuf-c library
#
function(c3qo_add_library_protobuf_c target_name proto_dir proto_prefix)

    set(proto_src_dir ${CMAKE_CURRENT_SOURCE_DIR}/${proto_dir})
    set(proto_src_file ${proto_prefix}.proto)

    set(proto_gen_dir ${CMAKE_CURRENT_BINARY_DIR}/${proto_dir})
    set(proto_gen_file ${proto_gen_dir}/${proto_prefix}.pb-c.c)
    set(proto_gen_cmd protoc-c --c_out=${proto_gen_dir} ${proto_src_file})

    execute_process(COMMAND mkdir -p ${proto_gen_dir})
    add_custom_command(
        OUTPUT            ${proto_gen_file}
        COMMAND           ${proto_gen_cmd}
        WORKING_DIRECTORY ${proto_src_dir}
        DEPENDS           ${proto_src_dir}/${proto_src_file}
    )

    add_library(${target_name} STATIC ${proto_gen_file})
    target_link_libraries(${target_name} /usr/local/lib/libprotobuf-c.a)
    target_compile_options(${target_name} PRIVATE ${COMPILE_FLAGS_C})
    target_include_directories(${target_name} PUBLIC ${proto_gen_dir})
endfunction()
