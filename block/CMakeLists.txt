

add_subdirectory(hello)
add_subdirectory(project_euler)

# I did not manage to make it work on Windows platform yet
if (NOT (${CMAKE_SYSTEM_NAME} STREQUAL Windows))
    add_subdirectory(socket_us_nb)
    add_subdirectory(protobuf)
    add_subdirectory(socket_zmq)
endif()

