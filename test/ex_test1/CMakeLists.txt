cmake_minimum_required(VERSION 3.5.1)
project(ex_test1)

set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin )
set( SH_DATA_LIB /usr/local/lib/libshared_data.so)

add_executable(shmem1 src/shmem1.c)
target_link_libraries(shmem1 ${SH_DATA_LIB})

add_executable(shmem2 src/shmem2.c)
target_link_libraries(shmem2 ${SH_DATA_LIB})
