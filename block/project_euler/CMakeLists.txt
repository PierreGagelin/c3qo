

# Build block project_euler library
set(SOURCES src/project_euler.cpp)

add_library(project_euler ${C3QO_LIB_TYPE} ${SOURCES})

C3QO_TARGET_COMPILE_FLAGS(project_euler)

target_link_libraries(project_euler manager)

# Build problem solutions as a shared library (never static !)
set(SOURCES_LIB)
set(SOURCES_LIB ${SOURCES_LIB} src/problem_1.cpp)
set(SOURCES_LIB ${SOURCES_LIB} src/problem_2.cpp)
set(SOURCES_LIB ${SOURCES_LIB} src/problem_3.cpp)
set(SOURCES_LIB ${SOURCES_LIB} src/problem_51.cpp)
set(SOURCES_LIB ${SOURCES_LIB} src/problem.hpp)

add_library(project_euler_lib SHARED ${SOURCES_LIB})

C3QO_TARGET_COMPILE_FLAGS(project_euler_lib)
C3QO_TARGET_INCLUDE(project_euler_lib)

target_link_libraries(project_euler_lib logger)

# Build test tu_project_euler as an executable
set(SOURCES_TEST test/tu_project_euler.cpp)

add_executable(tu_project_euler ${SOURCES_TEST})

C3QO_TARGET_LINK_FLAGS(tu_project_euler)

add_test(NAME tu_project_euler COMMAND tu_project_euler)

target_link_libraries(tu_project_euler project_euler)
target_link_libraries(tu_project_euler project_euler_lib)
target_link_libraries(tu_project_euler gtest)
target_link_libraries(tu_project_euler gtest_main)

