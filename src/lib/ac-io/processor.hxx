//
// Copyright (c) 2026 Jamie Kenyon. All Rights Reserved.
//

#pragma once

#include "ac-io/job.hxx"
#include "ac-io/manifest.hxx"
#include <system_error>
#include <vector>

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

        /// Returns the results of jobs that failed during the most recent run().
        /// \return The failed job results.
        [[nodiscard]] auto failedJobs() const -> std::vector<JobResult> const&;

    private:
        std::vector<JobResult> _failedJobs; ///< Results of jobs that failed during the most recent run().
    };
} // namespace ac
