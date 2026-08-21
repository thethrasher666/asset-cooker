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
        ktxtools
        ktx2check
        ktx2ktx2
        ktxinfo
        ktxsc
        toktx

    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
)

# KTX-Software installs its own dev SDK files (headers, .lib/.a files, cmake
# package config) as a side effect of building its CLI tools, and there's no
# option to gate that off. Remove them post-install instead — asset-cooker
# never installs anything of its own into include/ or lib/.
install(CODE "
    file(REMOVE_RECURSE
        \"\${CMAKE_INSTALL_PREFIX}/include\"
        \"\${CMAKE_INSTALL_PREFIX}/lib\"
    )
")
