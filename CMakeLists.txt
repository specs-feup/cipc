cmake_minimum_required(VERSION 3.20.0)

project(cipc)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_C_FLAGS_RELEASE "-Wall -Wextra -Werror")

set(SRC_DIR ${CMAKE_SOURCE_DIR}/src)
set(INC_DIR ${CMAKE_SOURCE_DIR}/include)
set(EXAMPLES_DIR ${CMAKE_SOURCE_DIR}/examples)

include_directories(${INC_DIR} ${INC_DIR}/backend)

add_library(cipc_lib STATIC ${SRC_DIR}/cipc.c ${SRC_DIR}/backend/cipc_zmq.c)

# main target
add_executable(cipc ${SRC_DIR}/main.c)
target_include_directories(cipc PUBLIC ${INC_DIR})
target_link_libraries(cipc cipc_lib)

# examples target
add_executable(cipc_client ${EXAMPLES_DIR}/zmq/cipc_zmq_client.c)
target_include_directories(cipc_client PRIVATE ${INC_DIR})
target_link_libraries(cipc_client cipc_lib zmq)

add_executable(cipc_server ${EXAMPLES_DIR}/zmq/cipc_zmq_server.c)
target_include_directories(cipc_server PRIVATE ${INC_DIR})
target_link_libraries(cipc_server cipc_lib zmq)
