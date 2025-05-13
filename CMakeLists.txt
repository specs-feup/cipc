cmake_minimum_required(VERSION 3.20.0)

project(cipc LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED TRUE)
set(CMAKE_C_FLAGS_RELEASE "-Wall -Wextra -Werror")

set(SRC_DIR ${CMAKE_SOURCE_DIR}/src)
set(INC_DIR ${CMAKE_SOURCE_DIR}/include)
set(EXAMPLES_DIR ${CMAKE_SOURCE_DIR}/examples)

# Library
add_library(cipc STATIC
    ${SRC_DIR}/cipc.c
    ${SRC_DIR}/backend/cipc_zmq.c
)
set_target_properties(cipc PROPERTIES OUTPUT_NAME "cipc")
target_include_directories(cipc PUBLIC ${INC_DIR})

# Install headers and library
install(DIRECTORY ${INC_DIR}/ DESTINATION include)
install(TARGETS cipc
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

# Main binary
add_executable(cipc_main ${SRC_DIR}/main.c)
target_include_directories(cipc_main PRIVATE ${INC_DIR})
target_link_libraries(cipc_main cipc)

# Example binaries
set(EXAMPLES_ZMQ ${EXAMPLES_DIR}/zmq)
set(EXAMPLES_TCP ${EXAMPLES_DIR}/tcp)

add_executable(example_zmq_req ${EXAMPLES_ZMQ}/cipc_zmq_req.c)
add_executable(example_zmq_rep ${EXAMPLES_ZMQ}/cipc_zmq_rep.c)

target_include_directories(example_zmq_req PRIVATE ${INC_DIR})
target_include_directories(example_zmq_rep PRIVATE ${INC_DIR})

target_link_libraries(example_zmq_req cipc zmq)
target_link_libraries(example_zmq_rep cipc zmq)
