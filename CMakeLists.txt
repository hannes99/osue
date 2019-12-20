cmake_minimum_required(VERSION 3.15)
project(osue C)

set(CMAKE_C_STANDARD 99)

add_executable(mydiff 1A/mydiff.c)
add_executable(server 1B/server.c 1B/http.c 1B/http.h 1B/url.c 1B/url.h)
add_executable(client 1B/client.c 1B/http.c 1B/http.h 1B/url.c 1B/url.h)
add_executable(forksortx 2/forksort.c 2/logger.h 2/logger.c)
add_executable(test Testing/Test.c)

#gitConfig ()
{
    case "$1" in
                        "1")
                            git config user.name "Hannes Laimer"
                            git config user.email "hannes.laimer@rolmail.net"
                            git config user.signingkey "E8E6250F2E9DFE2EC496E0F26661BDAE4F07A91F"
                            ;;
                        "2")
                            git config user.name "Hannes Laimer"
                            git config user.email "e11808227@student.tuwien.ac.at"
                            git config user.signingkey "FEE4CE8BBDB4F0969A2215D28D2538D8F4AC6D2C"
                            ;;
                        "3")
                            echo "Cinfig 3"
                            ;;
                        *)
                            echo "no valid config"
                            ;;
          esac
    git config user.name
    git config user.email
    git config user.signingkey
}

