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

# GLM
FetchContent_Declare(glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG        1.0.3
    GIT_SHALLOW    TRUE
    EXCLUDE_FROM_ALL
    SYSTEM
)
FetchContent_MakeAvailable(glm)

# KTX Software.
# set(KTX_FEATURE_TESTS OFF CACHE BOOL "Create unit tests.")
# set(KTX_FEATURE_TOOLS ON  CACHE BOOL "Enable KTX CLI Tools.")
# 
# FetchContent_Declare(KtxSoftware
#     GIT_REPOSITORY https://github.com/KhronosGroup/KTX-Software.git
#     GIT_TAG        v5.0.0-rc2
#     GIT_SHALLOW    TRUE
#     EXCLUDE_FROM_ALL
#     SYSTEM
# )
# set(CMAKE_SKIP_INSTALL_RULES TRUE)
# FetchContent_MakeAvailable(KtxSoftware)
# unset(CMAKE_SKIP_INSTALL_RULES)

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
set(TINYGLTF3_BUILD_TESTS OFF CACHE BOOL "Build and run tinygltf v3 C unit tests")
set(TINYGLTF3_INSTALL     OFF CACHE BOOL "Install tinygltf v3 files during install step")

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
target_compile_definitions(TinyGLTF PRIVATE TINYGLTF3_ENABLE_FS)
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
