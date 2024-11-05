# CMakeLists.txt inside libs/unrar-free

# Create a static library from unrarlib.c
add_library(unrar-free STATIC unrarlib.c)

# Optionally, specify the include directory for this library
target_include_directories(unrar-free PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
