#
# Copyright (c) 2026 Jamie Kenyon. All Rights Reserved.
#

include("FetchContent")

# Catch2.
FetchContent_Declare(Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG        v3.15.2
    GIT_SHALLOW    TRUE
    EXCLUDE_FROM_ALL
    SYSTEM
)
FetchContent_MakeAvailable(Catch2)

# Doxygen.
if(AC_BUILD_DOCS)
    find_package(Doxygen REQUIRED)
endif()

# KTX Software.
set(KTX_FEATURE_TESTS OFF CACHE BOOL "Create unit tests.")
set(BUILD_TESTING OFF CACHE BOOL "Build the testing tree.")

FetchContent_Declare(KtxSoftware
    GIT_REPOSITORY https://github.com/KhronosGroup/KTX-Software.git
    GIT_TAG        v4.4.2
    GIT_SHALLOW    TRUE
    EXCLUDE_FROM_ALL
    SYSTEM
)
FetchContent_MakeAvailable(KtxSoftware)
set(BUILD_TESTING ON CACHE BOOL "Build the testing tree.")

# Mesh Optimizer.
set(MESHOPT_INSTALL OFF CACHE BOOL "Install library")

FetchContent_Declare(MeshOptimizer
    GIT_REPOSITORY https://github.com/zeux/meshoptimizer.git
    GIT_TAG        v1.2
    GIT_SHALLOW    TRUE
    EXCLUDE_FROM_ALL
    SYSTEM
)
FetchContent_MakeAvailable(MeshOptimizer)

# Pak I/O.
FetchContent_Declare(PakIO
    GIT_REPOSITORY https://github.com/thethrasher666/pak-io.git
    GIT_TAG        main
    GIT_SHALLOW    TRUE
    EXCLUDE_FROM_ALL
    SYSTEM
)
FetchContent_MakeAvailable(PakIO)

# Process Library.
FetchContent_Declare(ProcLib
    GIT_REPOSITORY https://github.com/thethrasher666/proc-lib.git
    GIT_TAG        main
    GIT_SHALLOW    TRUE
    EXCLUDE_FROM_ALL
    SYSTEM
)
FetchContent_MakeAvailable(ProcLib)

# TinyGLTF.
FetchContent_Declare(TinyGLTF
    GIT_REPOSITORY https://github.com/syoyo/tinygltf.git
    GIT_TAG        v3.0.1
    GIT_SHALLOW    TRUE
    EXCLUDE_FROM_ALL
    SYSTEM
)
FetchContent_MakeAvailable(TinyGLTF)
add_library(TinyGLTF "${tinygltf_SOURCE_DIR}/tiny_gltf_v3.c")
add_library(TinyGLTF::TinyGLTF ALIAS TinyGLTF)
target_include_directories(TinyGLTF PUBLIC "${tinygltf_SOURCE_DIR}")

# TOML++.
FetchContent_Declare(TomlPlusPlus
    GIT_REPOSITORY https://github.com/marzer/tomlplusplus.git
    GIT_TAG        v3.4.0
    GIT_SHALLOW    TRUE
    EXCLUDE_FROM_ALL
    SYSTEM
)
FetchContent_MakeAvailable(TomlPlusPlus)

# Vulkan SDK.
find_package(Vulkan REQUIRED)
