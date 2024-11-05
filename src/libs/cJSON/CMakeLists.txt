# CMakeLists.txt inside libs/cJSON

# Create a static library from cJSON.c
add_library(cJSON STATIC cJSON.c)

# Specify the include directory for this library
target_include_directories(cJSON PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
