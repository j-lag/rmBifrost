cmake_minimum_required(VERSION 3.10)
project(rmBifrost_project)
set(CMAKE_CXX_STANDARD 17)

include(cmake/CMakeRC.cmake)

include_directories(include)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

add_subdirectory(src)
add_subdirectory(external)


