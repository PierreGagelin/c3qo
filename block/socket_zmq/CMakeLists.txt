

# Build pub_sub as a static library
set(SOURCES_PUB_SUB src/pub_sub.cpp)

add_library(pub_sub STATIC ${SOURCES_PUB_SUB})

C3QO_TARGET_COMPILE_FLAGS(pub_sub)

target_link_libraries(pub_sub manager_bk)


# Build TU for pub_sub
set(SOURCES_TU test/tu_pub_sub.cpp)

add_executable(tu_pub_sub ${SOURCES_TU})

C3QO_TARGET_LINK_FLAGS(tu_pub_sub)

add_test(NAME tu_pub_sub COMMAND tu_pub_sub)


target_link_libraries(tu_pub_sub pub_sub)
target_link_libraries(tu_pub_sub gtest)
target_link_libraries(tu_pub_sub gtest_main)


