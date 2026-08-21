#
# Copyright (c) 2026 Jamie Kenyon. All Rights Reserved.
#

# Configure files.
configure_file("version.hxx.in" "version.hxx")

# Define the target.
add_library(ac-io STATIC
    "error.cxx"
    "job.cxx"
    "manifest.cxx"
    "mesh-processor.cxx"
    "processor.cxx"
    "texture-processor.cxx"
)

# Set the target defaults.
ac_set_target_defaults(ac-io)

# Set the preprocessor definitions.
target_compile_definitions(ac-io
    PRIVATE
        TOML_EXCEPTIONS=0
)

# Set the target's include directories.
target_include_directories(ac-io
    PUBLIC
        "${CMAKE_CURRENT_BINARY_DIR}/../"
        "${CMAKE_CURRENT_SOURCE_DIR}/../"
)

# Link the dependent libraries.
target_link_libraries(ac-io
    PUBLIC
        proc-lib::proc-lib
        tomlplusplus::tomlplusplus
        TinyGLTF::TinyGLTF
)

# Add an alias for the target.
add_library(ac-io::ac-io ALIAS ac-io)
