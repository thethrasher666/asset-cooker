//
// Copyright (c) 2026 Jamie Kenyon. All Rights Reserved.
//

#include "ac-io/error.hxx"

#include <format>

namespace ac
{
    namespace
    {
        /// Custom error category for error codes.
        class ErrorCategory final : public std::error_category
        {
        public:
            /// Inherited from std::error_category.
            [[nodiscard]] auto name() const noexcept -> char const* final
            {
                return "asset-cook::category";
            }

            /// Inherited from std::error_category.
            [[nodiscard]] auto message(int32_t value) const -> std::string final
            {
                auto const errorCode{ static_cast<ErrorCode>(value) };

                switch (errorCode)
                {
                case ErrorCode::NoError:
                    return "No error has occurred.";

                case ErrorCode::ManifestParseFailed:
                    return "The manifest could not be parsed as TOML.";

                case ErrorCode::InvalidVersionIdentifier:
                    return "The version identifier in the manifest is invalid.";

                case ErrorCode::ToolchainVersionTooOld:
                    return "The toolchain version is too old to read or write the PAK file.";

                case ErrorCode::AssetProcessingFailed:
                    return "One or more assets failed to process.";

                case ErrorCode::GltfParseFailed:
                    return "The glTF/GLB asset could not be parsed.";

                case ErrorCode::MeshDataEmpty:
                    return "The asset contains no renderable mesh data.";

                case ErrorCode::MeshMissingPositionAttribute:
                    return "A mesh primitive has no POSITION attribute.";

                case ErrorCode::MeshUnsupportedPrimitiveMode:
                    return "A mesh primitive uses a mode other than triangles, triangle strip, or triangle fan.";

                case ErrorCode::MeshInvalidAccessorData:
                    return "A mesh primitive references invalid or out-of-range accessor, buffer view, or buffer data.";

                case ErrorCode::OutputFileCreationFailed:
                    return "An output file failed to create.";

                case ErrorCode::ProcessFailedToStart:
                    return "The process could not be started.";

                case ErrorCode::ProcessNonZeroExit:
                    return "The process exited with a non-zero code.";

                case ErrorCode::ProcessTerminated:
                    return "The process was terminated.";

                case ErrorCode::ProcessKilled:
                    return "The process was killed.";

                case ErrorCode::ProcessCrashed:
                    return "The process crashed.";

                case ErrorCode::ProcessFailed:
                    return "The process encountered an error.";
                }

                return "No error has occurred.";
            }
        };

        [[nodiscard]] auto category() -> std::error_category const&
        {
            static ErrorCategory instance;
            return instance;
        }
    } // namespace

    auto makeErrorCode(ErrorCode const code) -> std::error_code
    {
        return std::error_code(static_cast<int32_t>(code), category());
    }
} // namespace ac
