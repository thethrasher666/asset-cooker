#
# Copyright (c) 2026 Jamie Kenyon. All Rights Reserved.
#

#set_tests_properties(tok-test PROPERTIES DISABLED True)
#set_tests_properties(ktxsc-test PROPERTIES DISABLED True)
#set_tests_properties(ktx2ktx2-test PROPERTIES DISABLED True)
#set_tests_properties(ktx2check-test PROPERTIES DISABLED True)

# Define the target.
add_executable(test_asset_cooker
    "test-manifest.cxx"
)

# Set the defaults.
ac_set_target_defaults(test_asset_cooker)

# Set the target's include directories.
target_include_directories(test_asset_cooker
    PRIVATE
        "${CMAKE_SOURCE_DIR}/src"
)

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
