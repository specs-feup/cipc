cmake_minimum_required(VERSION 3.20.0)

project(cipc)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_C_FLAGS_RELEASE "-Wall -Wextra -Werror")

include_directories(include)

add_executable(cipc src/main.c)

target_include_directories(cipc PUBLIC include)
