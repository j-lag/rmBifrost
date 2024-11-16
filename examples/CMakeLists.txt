add_subdirectory(example_app)

# Add a custom target to build the example app
add_custom_target(examples)
add_dependencies(examples example_app)