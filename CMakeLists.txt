cmake_minimum_required(VERSION 3.26)
project(z2 C)

set(CMAKE_C_STANDARD 17)

add_executable(z2 shell.c
        client_lib.c
        server_lib.c)
