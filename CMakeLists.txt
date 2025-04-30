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
set(EXAMPLES_ZMQ ${EXAMPLES_DIR}/zmq)
set(EXAMPLES_TCP ${EXAMPLES_DIR}/tcp)

add_executable(example_zmq_req ${EXAMPLES_ZMQ}/cipc_zmq_req.c)
add_executable(example_zmq_rep ${EXAMPLES_ZMQ}/cipc_zmq_rep.c)

target_include_directories(example_zmq_req PRIVATE ${INC_DIR})
target_include_directories(example_zmq_rep PRIVATE ${INC_DIR})

target_link_libraries(example_zmq_req cipc_lib zmq)
target_link_libraries(example_zmq_rep cipc_lib zmq)
