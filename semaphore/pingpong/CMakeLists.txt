cmake_minimum_required(VERSION 3.10)

project(IPC_Subtraction C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_compile_options(-Wall -Wextra)

add_executable(init init.c common.h)

# Собираем worker из worker.c

add_custom_command(TARGET init POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E touch ${CMAKE_BINARY_DIR}/keyfile
    COMMENT "Creating 'keyfile' for ftok generation"
)