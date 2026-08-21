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
        NoError,                  ///< No error has occurred.
        ManifestParseFailed,      ///< The manifest could not be parsed as TOML.
        InvalidVersionIdentifier, ///< The version identifier in the manifest is invalid.
        ToolchainVersionTooOld,   ///< The toolchain version is too old to read or write the PAK file.
        AssetProcessingFailed,    ///< One or more assets failed to process.
    };

    /// Create an error code.
    /// \param code The error code.
    /// \return A valid error code.
    [[nodiscard]] auto makeErrorCode(ErrorCode const code) -> std::error_code;
} // namespace ac
