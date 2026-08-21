#
# Copyright (c) 2026 Jamie Kenyon. All Rights Reserved.
#

# Define the target.
add_executable(test_asset_cooker
    "test-manifest.cxx"
    "test-mesh-cooker.cxx"
    "test-mesh-writer.cxx"
)

# Set the defaults.
ac_set_target_defaults(test_asset_cooker)

# Set the target's compile definitions.
target_compile_definitions(test_asset_cooker
    PRIVATE
        AC_TEST_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data"
)

# Set the target's dependencies.
target_link_libraries(test_asset_cooker
    PRIVATE
        ac-io::ac-io
        Catch2::Catch2WithMain
)

# Discover the tests.
catch_discover_tests(test_asset_cooker
    DISCOVERY_MODE
        PRE_TEST
)

# Test the make tool.
add_test(
    NAME
        test.ac.tool.execution
    COMMAND
        $<TARGET_FILE:asset-cooker> "${CMAKE_CURRENT_SOURCE_DIR}/data/manifest.toml"
)
