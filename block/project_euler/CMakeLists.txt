

# Build block project_euler library
set(SOURCES)
set(SOURCES ${SOURCES} src/project_euler.cpp)
set(SOURCES ${SOURCES} src/problem_1.cpp)
set(SOURCES ${SOURCES} src/problem_2.cpp)
set(SOURCES ${SOURCES} src/problem_3.cpp)
set(SOURCES ${SOURCES} src/problem_51.cpp)
set(SOURCES ${SOURCES} src/problem.hpp)

add_library(project_euler SHARED ${SOURCES})

C3QO_TARGET_COMPILE_FLAGS(project_euler)

target_link_libraries(project_euler manager)


# Build test tu_project_euler as an executable
set(SOURCES_TEST test/tu_project_euler.cpp)

add_executable(tu_project_euler ${SOURCES_TEST})

C3QO_TARGET_LINK_FLAGS(tu_project_euler)

add_test(NAME tu_project_euler COMMAND tu_project_euler)

target_link_libraries(tu_project_euler project_euler)
target_link_libraries(tu_project_euler gtest)
target_link_libraries(tu_project_euler gtest_main)

