set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(HEADER_LIST "${PROJECT_SOURCE_DIR}/include/bifrost/bifrost.hpp")

file(GLOB BSWR_SOURCES ${PROJECT_SOURCE_DIR}/external/BSWR/*.c)
file(GLOB_RECURSE lvgl_sources ${PROJECT_SOURCE_DIR}/external/lvgl/src/*.c ${PROJECT_SOURCE_DIR}/external/lvgl/src/*.cpp)

# -Wnarrowing
add_compile_options(-Wno-narrowing)
add_compile_definitions(LV_LVGL_H_INCLUDE_SIMPLE)

add_library(rmBifrost SHARED entrypoint.cpp
        bifrost.cpp
        bifrost_impl.cpp
        bifrost_impl.h
        BookConfig.cpp
        BookConfig.h
        gui/lvgl_renderer.cpp
        gui/lvgl_renderer.h
        ${BSWR_SOURCES}
        ${lvgl_sources}
        gui/boot_screen.cpp
        gui/boot_screen.h
        gui/ChooseFile_screen.cpp
        gui/ChooseFile_screen.h
        gui/ComicArchive.cpp
        gui/ComicArchive.h
        gui/ImageIo.cpp
        gui/ImageIo.h        
        # fonts
        resources/fonts/ebgaramond_48.c
)

target_precompile_headers(rmBifrost PRIVATE "gui/lv_conf.h")

set_target_properties(rmBifrost PROPERTIES OUTPUT_NAME "bifrost")

# import all Qt6 components
find_package(Qt6 COMPONENTS Core Gui REQUIRED)
target_link_libraries(rmBifrost PRIVATE Qt6::Core Qt6::Gui)
target_include_directories(rmBifrost PRIVATE ${Qt6Core_INCLUDE_DIRS} ${Qt6Gui_INCLUDE_DIRS})

add_subdirectory(resources)
target_link_libraries(rmBifrost PRIVATE bifrost::resources)

add_subdirectory(libs/miniz)
target_link_libraries(rmBifrost PRIVATE miniz)

add_subdirectory(libs/unrar-free)
target_link_libraries(rmBifrost PRIVATE unrar-free)

add_subdirectory(libs/cJSON)
target_link_libraries(rmBifrost PRIVATE cJSON)


## Find the libraries in paths
find_library(PNG_LIBRARY png PATHS )
find_library(ZLIB_LIBRARY z PATHS )
find_library(TURBOJPEG_LIB turbojpeg PATHS)

# Check if libraries are found
if (NOT TURBOJPEG_LIB)
    message(FATAL_ERROR "TurboJPEG library not found")
endif()
if(NOT PNG_LIBRARY)
    message(FATAL_ERROR "libpng not found in ${SYSROOT_PATH}/usr/lib")
endif()
if(NOT ZLIB_LIBRARY)
    message(FATAL_ERROR "libz not found in ${SYSROOT_PATH}/usr/lib")
endif()

## Link libraries to your target
target_link_libraries(rmBifrost PRIVATE ${PNG_LIBRARY} ${ZLIB_LIBRARY} ${TURBOJPEG_LIB} )
