#
# Build JSON utils
#

set(JSMN_DIR_DOWNLOAD ${C3QO_EXTERNAL_PROJECT_DIR}/jsmn/download)
set(JSMN_DIR_SOURCE ${C3QO_EXTERNAL_PROJECT_DIR}/jsmn/src)
set(JSMN_DIR_BINARY ${C3QO_EXTERNAL_PROJECT_DIR}/jsmn/src)

# Configure jsmn as an external project and build it (download)
configure_file(external_project_jsmn.cmake ${JSMN_DIR_DOWNLOAD}/CMakeLists.txt)
execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" . WORKING_DIRECTORY ${JSMN_DIR_DOWNLOAD})
execute_process(COMMAND ${CMAKE_COMMAND} --build                 . WORKING_DIRECTORY ${JSMN_DIR_DOWNLOAD})

# Define a target to compile jsmn
add_custom_target(jsmn_build COMMAND make WORKING_DIRECTORY ${JSMN_DIR_SOURCE})

# Retrieve jsmn library
add_library(jsmn STATIC IMPORTED)
set_property(TARGET jsmn PROPERTY IMPORTED_LOCATION ${JSMN_DIR_BINARY}/libjsmn.a)
add_dependencies(jsmn jsmn_build)

# Build json wrapper aroung jsmn
c3qo_add_library(json src/json.cpp)
target_link_libraries(json jsmn)
target_link_libraries(json logger)
target_include_directories(json PRIVATE ${JSMN_DIR_SOURCE})

# Build a test unit using json wrapper
if (${C3QO_TEST})
    c3qo_add_test(tu_json test/tu_json.cpp)
    target_link_libraries(tu_json json)
endif ()
