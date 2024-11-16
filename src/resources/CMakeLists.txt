# Recursively find all files in the resources directory
file(GLOB_RECURSE BIFROST_RESOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ./*)

# Exclude all *.c files
list(FILTER BIFROST_RESOURCES EXCLUDE REGEX "\\.c$")

# Remove the CMakeLists.txt file from the list of resources
list(REMOVE_ITEM BIFROST_RESOURCES "CMakeLists.txt")

cmrc_add_resource_library(bifrost_resources ALIAS bifrost::resources NAMESPACE bifrost
    ${BIFROST_RESOURCES}
)