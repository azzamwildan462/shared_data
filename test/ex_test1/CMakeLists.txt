cmake_minimum_required(VERSION 3.5.1)
project(ex_test1)

set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin )

add_executable(shmem1 src/shmem1.c)
target_link_libraries(shmem1 shared_data)

add_executable(shmem2 src/shmem2.c)
target_link_libraries(shmem2 shared_data)
