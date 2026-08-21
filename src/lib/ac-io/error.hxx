//
// Copyright (c) 2026 Jamie Kenyon. All Rights Reserved.
//

#pragma once

#include <string>
#include <system_error>

namespace ac
{
    /// The possible errors relating to cooking.
    enum class ErrorCode
    {
        NoError,                      ///< No error has occurred.
        ManifestParseFailed,          ///< The manifest could not be parsed as TOML.
        InvalidVersionIdentifier,     ///< The version identifier in the manifest is invalid.
        ToolchainVersionTooOld,       ///< The toolchain version is too old to read or write the PAK file.
        AssetProcessingFailed,        ///< One or more assets failed to process.
        GltfParseFailed,              ///< The glTF/GLB asset could not be parsed.
        MeshDataEmpty,                ///< The asset contains no renderable mesh data.
        MeshMissingPositionAttribute, ///< A mesh primitive has no POSITION attribute.
        MeshUnsupportedPrimitiveMode, ///< A mesh primitive uses a mode other than triangles, triangle strip, or triangle fan.
        MeshInvalidAccessorData,      ///< A mesh primitive references invalid or out-of-range accessor, buffer view, or buffer data.
        OutputFileCreationFailed,     ///< Failed to create an output file.
        ProcessFailedToStart,         ///< A process could not be started.
        ProcessNonZeroExit,           ///< A process exited with a non-zero code.
        ProcessTerminated,            ///< A process was terminated.
        ProcessKilled,                ///< A process was killed.
        ProcessCrashed,               ///< A process crashed.
        ProcessFailed,                ///< A process encountered an otherwise-unclassified error.
    };

    /// Create an error code.
    /// \param code The error code.
    /// \return A valid error code.
    [[nodiscard]] auto makeErrorCode(ErrorCode const code) -> std::error_code;
} // namespace ac
