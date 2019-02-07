

# Include directories for test units
set(TU_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/engine/test/include)

# Compilation flags
set(CMAKE_CXX_STANDARD 11)

set(COMPILE_FLAGS_COMMON)
set(COMPILE_FLAGS_COMMON ${COMPILE_FLAGS_COMMON} -Wall)
set(COMPILE_FLAGS_COMMON ${COMPILE_FLAGS_COMMON} -Wextra)
set(COMPILE_FLAGS_COMMON ${COMPILE_FLAGS_COMMON} -fno-rtti)

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
    endif()
endfunction()

#
# Add link flags
#
function (c3qo_target_link_flags t_name)
    # Required for ZeroMQ runtime
    target_link_libraries(${t_name} ${C3QO_ZEROMQ_BUILD}/lib/libzmq.so)

    # Add coverage link flag
    if (${C3QO_COVERAGE})
        set_property(TARGET ${t_name} APPEND_STRING PROPERTY LINK_FLAGS " --coverage")
    endif()
endfunction()

#
# Add a library
#
function (c3qo_add_library target_name target_sources)
    add_library(${target_name} STATIC ${target_sources})

    target_include_directories(${target_name} PUBLIC ${CMAKE_SOURCE_DIR}/utils/include/)
    c3qo_target_compile_flags(${target_name})
    c3qo_target_link_flags(${target_name})
endfunction()

#
# Add a block
#
function (c3qo_add_block target_name target_sources)
    c3qo_add_library(${target_name} "${target_sources}")

    target_link_libraries(${target_name} manager)
endfunction()

#
# Add an executable
#
function (c3qo_add_executable target_name target_sources)
    add_executable(${target_name} ${target_sources})

    c3qo_target_compile_flags(${target_name})
    c3qo_target_link_flags(${target_name})

    target_link_libraries(${target_name} logger)

    install(TARGETS ${target_name} DESTINATION bin)
endfunction()

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
endfunction()

#
# Add a protobuf-c library
#
function(c3qo_add_library_protobuf_c target_name proto_dir proto_prefix)

    set(proto_src_dir ${CMAKE_CURRENT_SOURCE_DIR}/${proto_dir})
    set(proto_src_file ${proto_prefix}.proto)

    set(proto_cmd ${C3QO_PROTOBUF}/protoc-c/protoc-c)
    set(proto_lib ${C3QO_PROTOBUF}/protobuf-c/.libs/libprotobuf-c.so)

    set(proto_gen_dir ${CMAKE_CURRENT_BINARY_DIR}/${proto_dir})
    set(proto_gen_file ${proto_gen_dir}/${proto_prefix}.pb-c.c)
    set(proto_gen_cmd ${proto_cmd} --c_out=${proto_gen_dir} ${proto_src_file})

    execute_process(COMMAND mkdir -p ${proto_gen_dir})
    add_custom_command(
        OUTPUT            ${proto_gen_file}
        COMMAND           ${proto_gen_cmd}
        WORKING_DIRECTORY ${proto_src_dir}
        DEPENDS           ${proto_src_dir}/${proto_src_file}
    )

    add_library(${target_name} STATIC ${proto_gen_file})
    target_link_libraries(${target_name} ${proto_lib})
    target_compile_options(${target_name} PRIVATE ${COMPILE_FLAGS_C})
    target_include_directories(${target_name} PUBLIC ${proto_gen_dir})
    target_include_directories(${target_name} PUBLIC ${C3QO_PROTOBUF})
endfunction()
