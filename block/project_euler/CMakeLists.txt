

# Build block project_euler library
set(SOURCES src/project_euler.cpp)

c3qo_add_block(project_euler "${SOURCES}")


# Build problem solutions as a shared library
set(SOURCES_LIB)
set(SOURCES_LIB ${SOURCES_LIB} src/problem_1.cpp)
set(SOURCES_LIB ${SOURCES_LIB} src/problem_2.cpp)
set(SOURCES_LIB ${SOURCES_LIB} src/problem_3.cpp)
set(SOURCES_LIB ${SOURCES_LIB} src/problem_51.cpp)
set(SOURCES_LIB ${SOURCES_LIB} src/problem.hpp)

add_library(project_euler_lib SHARED ${SOURCES_LIB})

c3qo_target_compile_flags(project_euler_lib)
c3qo_target_link_flags(project_euler_lib)
c3qo_target_include(project_euler_lib)

target_link_libraries(project_euler_lib logger)


# Build test tu_project_euler as an executable
set(SOURCES_TU test/tu_project_euler.cpp)

c3qo_add_test(tu_project_euler "${SOURCES_TU}")

target_link_libraries(tu_project_euler project_euler)
target_link_libraries(tu_project_euler project_euler_lib)

