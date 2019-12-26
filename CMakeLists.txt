cmake_minimum_required(VERSION 3.15)
project(osue C)

set(CMAKE_C_STANDARD 99)

add_executable(mydiff 1A/mydiff.c)
add_executable(server 1B/server.c 1B/http.c 1B/http.h 1B/url.c 1B/url.h)
add_executable(client 1B/client.c 1B/http.c 1B/http.h 1B/url.c 1B/url.h)
add_executable(forksort 2/forksort.c 2/logger.h 2/logger.c)
add_executable(generator 3/generator.c 3/edge.c 3/edge.h 3/logger.c 3/logger.h 3/shared.c 3/shared.h)
add_executable(supervisor 3/supervisor.c 3/edge.c 3/edge.h 3/logger.c 3/logger.h 3/shared.c 3/shared.h)
add_executable(test 3/test.c 3/logger.c 3/logger.h 3/shared.c 3/shared.h)

find_library(LIBRT rt)
if (LIBRT)
    target_link_libraries(supervisor ${LIBRT} -pthread)
    target_link_libraries(test ${LIBRT} -pthread)
    target_link_libraries(generator ${LIBRT} -pthread)
endif ()