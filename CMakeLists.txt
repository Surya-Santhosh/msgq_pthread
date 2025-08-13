cmake_minimum_required(VERSION 3.10)
set(CMAKE_C_STANDARD 17)
project(MSGQ)

# Add the folders for header files
include_directories(${CMAKE_SOURCE_DIR}/)

add_executable(MSGQ_GCC 
               ${CMAKE_SOURCE_DIR}/main.c
               ${CMAKE_SOURCE_DIR}/multiThread/multiThread.c)
              

# List executable files in release folder

target_compile_options(MSGQ_GCC PRIVATE 
                       -Wall -Wextra -Werror -fanalyzer)

set_target_properties(MSGQ_GCC PROPERTIES 
                      RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/release)

# List debug files in debug folder
add_executable(MSGQ_Debug 
               ${CMAKE_SOURCE_DIR}/main.c
               ${CMAKE_SOURCE_DIR}/multiThread/multiThread.c)

target_compile_options(MSGQ_Debug PRIVATE -g -O0 -c -fanalyzer)

set_target_properties(MSGQ_Debug PROPERTIES 
                      RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/debug)
