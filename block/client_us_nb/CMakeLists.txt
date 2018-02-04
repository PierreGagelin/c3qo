

set(SOURCES src/client_us_nb.cpp)


add_library(client_us_nb STATIC ${SOURCES})


target_include_directories(client_us_nb PUBLIC ${C3QO_INCLUDE_DIR})
target_include_directories(client_us_nb PUBLIC ${UTILS_INCLUDE_DIR})
target_include_directories(client_us_nb PUBLIC ${UTILS_INCLUDE_DIR})


target_link_libraries(client_us_nb socket)

