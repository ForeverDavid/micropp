

cmake_minimum_required(VERSION 3.0)

project(MicroPP VERSION 0.1
  LANGUAGES C CXX Fortran)

# Use C++11
if (CMAKE_VERSION VERSION_LESS "3.1")
  if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11")
  endif ()
else ()
  set (CMAKE_CXX_STANDARD 11)
endif ()

# Include Directories (for all targets)
include_directories(include)

# wildcard all the sources in src
file(GLOB SOURCESLIB src/*.cpp)

# Library
add_library(micropp ${SOURCESLIB})

# Enable auto create tests
enable_testing()
add_subdirectory(test)
