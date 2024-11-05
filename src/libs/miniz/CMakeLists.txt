# CMakeLists.txt inside libs/miniz

# Create a static library from miniz.c
add_library(miniz STATIC miniz.c)

# Specify the include directory for this library
target_include_directories(miniz PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
