

# Build problem solutions
set(SOURCES_LIB)
set(SOURCES_LIB ${SOURCES_LIB} src/problem_1.cpp)
set(SOURCES_LIB ${SOURCES_LIB} src/problem_2.cpp)
set(SOURCES_LIB ${SOURCES_LIB} src/problem_3.cpp)
set(SOURCES_LIB ${SOURCES_LIB} src/problem_51.cpp)
set(SOURCES_LIB ${SOURCES_LIB} src/problem.hpp)

c3qo_add_library(project_euler_lib "${SOURCES_LIB}")


# Build block project_euler
c3qo_add_block(project_euler src/project_euler.cpp)
target_link_libraries(project_euler project_euler_lib)

if (${C3QO_TEST})
    # Build test tu_project_euler as an executable
    c3qo_add_test(tu_project_euler test/tu_project_euler.cpp)
    c3qo_link_block(tu_project_euler project_euler)
endif ()
