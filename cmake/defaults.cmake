#
# Copyright (c) 2026 Jamie Kenyon. All Rights Reserved.
#

# Standard install destinations (CMAKE_INSTALL_BINDIR, etc.).
include("GNUInstallDirs")

# Minimize the install messages.
set(CMAKE_INSTALL_MESSAGE LAZY)

# Default to static libraries. Claiming the cache entry here, before any
# dependencies are fetched, stops KTX-Software's own option(BUILD_SHARED_LIBS
# ...) call from silently switching every vendored dependency to shared.
option(BUILD_SHARED_LIBS "Build libraries as shared." OFF)

# Generate values from the current date that can be used in file configuration macros.
string(TIMESTAMP BUILD_YEAR  "%Y")
string(TIMESTAMP BUILD_MONTH "%m")
string(TIMESTAMP BUILD_DAY   "%d")
string(TIMESTAMP BUILD_TIME  "%H:%M:%S")

# Set default options on all target types.
function(ac_set_target_defaults TRGT)
    # Set properties.
    set_target_properties(${TRGT}
        PROPERTIES
            COMPILE_WARNING_AS_ERROR  ON
            C_VISIBILITY_PRESET       default
            CXX_VISIBILITY_PRESET     default
            POSITION_INDEPENDENT_CODE ON
            VISIBILITY_INLINES_HIDDEN OFF
    )

    # Set the target's compile definitions.
    target_compile_definitions(${TRGT}
        PRIVATE
            $<$<CXX_COMPILER_ID:MSVC>:_AMD64_>
            $<$<CXX_COMPILER_ID:MSVC>:NOMINMAX>
            $<$<CXX_COMPILER_ID:MSVC>:WIN32_LEAN_AND_MEAN>
    )

    # Set the C++ standard version.
    # PUBLIC so that exported targets impose the same standard on consumers.
    target_compile_features(${TRGT} PUBLIC cxx_std_23)

    # Set C++ flags.
    target_compile_options(${TRGT}
        PRIVATE
            # Set warnings as errors.
            "$<$<COMPILE_LANGUAGE:CXX>:$<$<CXX_COMPILER_ID:AppleClang,Clang,GNU>:-Wall>>"
            "$<$<COMPILE_LANGUAGE:CXX>:$<$<CXX_COMPILER_ID:MSVC>:/W4>>"
            "$<$<COMPILE_LANGUAGE:CXX>:$<$<AND:$<CXX_COMPILER_ID:MSVC>,$<CONFIG:Release>>:/Zi>>"
            "$<$<COMPILE_LANGUAGE:CXX>:$<$<AND:$<CXX_COMPILER_ID:AppleClang,GNU>,$<CONFIG:Release>>:-g>>"
    )
endfunction(ac_set_target_defaults)
