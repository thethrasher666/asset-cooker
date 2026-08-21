#
# Copyright (c) 2026 Jamie Kenyon. All Rights Reserved.
#

# Set the minimum CMake version.
cmake_minimum_required(VERSION 3.27.1 FATAL_ERROR)

# Define the project.
project(AssetCooker
    VERSION         0.1.0.0
    DESCRIPTION     "A cross-platform asset cooker for Vulkan."
    HOMEPAGE_URL    "https://www.jamiekenyon.com/"
    LANGUAGES       CXX C
)

# Include the dependent modules.
list(PREPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include("defaults")
include("options")
include("packages")

# Add the documentation.
if(AC_BUILD_DOCS)
    add_subdirectory("docs")
endif()

# Add the sources.
add_subdirectory("src")

# Enable testing.
if(AC_BUILD_TESTING)
    enable_testing()
    include("Catch")
    add_subdirectory("tests")
endif()

# Installation.
install(
    TARGETS
        asset-cooker

    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
)
