cmake_minimum_required(VERSION 3.5.1)
project(shared_data)

# Set output directories for libraries and executables
set( INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/include/)
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib )
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin )

include_directories(${INCLUDE_DIRS})


add_library(shared_data SHARED src/shared_data.c)
set_target_properties(shared_data PROPERTIES PUBLIC_HEADER include/shared_data.h)

# Install to system 
include(GNUInstallDirs)
install(TARGETS shared_data
        PUBLIC_HEADER
          DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/shared_data
)