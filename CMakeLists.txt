cmake_minimum_required(VERSION 3.10)
project(rmBifrost)
set(CMAKE_CXX_STANDARD 17)

include(cmake/CMakeRC.cmake)

include_directories(include)

add_compile_definitions(RM_VERSION_3_15_4_2)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Add cereal headers
include(FetchContent)
FetchContent_Declare(
        cereal
        GIT_REPOSITORY https://github.com/USCiLab/cereal.git
        GIT_TAG v1.3.2
)
FetchContent_MakeAvailable(cereal)

include_directories(${cereal_SOURCE_DIR}/include)

add_subdirectory(src)
add_subdirectory(external)
add_subdirectory(examples)

add_custom_target(rmBifrost ALL
    DEPENDS rmBifrost_client rmBifrost_compositor examples
)

