#
# Copyright (c) 2026 Jamie Kenyon. All Rights Reserved.
#

# Define the target.
add_executable(asset-cooker
    "main.cxx"
)

# Set the target defaults.
ac_set_target_defaults(asset-cooker)

# Link the dependent libraries.
target_link_libraries(asset-cooker
    PRIVATE
        ac-io::ac-io
)
