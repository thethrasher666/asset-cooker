//
// Copyright (c) 2026 Jamie Kenyon. All Rights Reserved.
//

#pragma once

#include "ac-io/job.hxx"

#include <filesystem>
#include <system_error>
#include <tiny_gltf_v3.h>

namespace ac
{
    /// Processes a given mesh into a binary format.
    class MeshProcessor final
    {
    public:
        /// Processes the mesh at the given path.
        /// \param path The path to the mesh file to be processed.
        /// \return An error code indicating the success or failure of the processing.
        [[nodiscard]] auto run(std::filesystem::path const& path) -> std::error_code;

        /// Builds a job that processes the mesh at the given path.
        /// \param path The path to the mesh file to be processed.
        /// \return A job that, when executed, processes the mesh.
        [[nodiscard]] auto toJob(std::filesystem::path path) -> Job;

    private:
        [[nodiscard]] auto processMeshFile(std::filesystem::path const& path) -> std::error_code;
    };
} // namespace ac
