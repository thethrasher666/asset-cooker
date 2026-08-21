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
