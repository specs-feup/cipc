cmake_minimum_required(VERSION 3.20.0)

project(cipc)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_C_FLAGS_RELEASE "-Wall -Wextra -Werror")

set(SRC_DIR ${CMAKE_SOURCE_DIR}/src)
set(INC_DIR ${CMAKE_SOURCE_DIR}/include)

include_directories(${INC_DIR} ${INC_DIR}/backend)

add_library(cipc_lib STATIC
    ${SRC_DIR}/cipc.c
)

add_executable(cipc src/main.c)

target_include_directories(cipc PUBLIC ${INC_DIR})

target_link_libraries(cipc cipc_lib)
