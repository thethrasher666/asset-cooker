//
// Copyright (c) 2026 Jamie Kenyon. All Rights Reserved.
//

#pragma once
#include "ac-io/manifest.hxx"
#include <system_error>

namespace ac
{
    /// The processor will process each file in the manifest and produce a PAK file of the processed assets.
    class Processor final
    {
    public:
        /// Runs the processor on the given manifest.
        /// \param manifest The manifest containing the files to be processed.
        /// \return An error code indicating the success or failure of the processing.
        [[nodiscard]] auto run(Manifest const& manifest) -> std::error_code;
    };
} // namespace ac
