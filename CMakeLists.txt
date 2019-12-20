cmake_minimum_required(VERSION 3.15)
project(osue C)

set(CMAKE_C_STANDARD 99)

add_executable(mydiff 1A/mydiff.c)
add_executable(server 1B/server.c 1B/http.c 1B/http.h 1B/url.c 1B/url.h)
add_executable(client 1B/client.c 1B/http.c 1B/http.h 1B/url.c 1B/url.h)
add_executable(forksortx 2/forksort.c 2/logger.h 2/logger.c)
add_executable(test Testing/Test.c)


