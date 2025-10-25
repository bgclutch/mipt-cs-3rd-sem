cmake_minimum_required(VERSION 3.10)

project(IPC_Carousel C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_compile_options(-Wall -Wextra)

add_executable(manager manager.c common.h)

add_executable(client client.c common.h)

add_custom_command(TARGET manager POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E touch ${CMAKE_BINARY_DIR}/keyfile
    COMMENT "Creating required 'keyfile' for IPC key generation..."
)