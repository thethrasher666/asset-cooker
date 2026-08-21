//
// Copyright (c) 2026 Jamie Kenyon. All Rights Reserved.
//

#pragma once

#include "ac-io/job.hxx"

#include <filesystem>
#include <system_error>

namespace ac
{
    /// The kind of PBR map a texture represents. Determines the KTX output format and encoding
    /// options used when converting it (e.g. sRGB for color data, linear for normal maps).
    enum class TextureKind
    {
        Albedo,           ///< Base color map.
        AmbientOcclusion, ///< Ambient occlusion map.
        Displacement,     ///< Displacement map.
        Emissive,         ///< Emissive map.
        Metallic,         ///< Metallic map.
        Normal,           ///< Normal map.
        Opacity,          ///< Opacity map.
        Roughness,        ///< Roughness map.
    };

    /// Processes a given texture using the bundled KTX programs.
    class TextureProcessor final
    {
    public:
        /// Processes the texture at the given path into KTX2 format.
        /// \param path The path to the texture file to be processed.
        /// \param kind The kind of PBR map the texture represents.
        /// \return An error code indicating the success or failure of the processing.
        [[nodiscard]] auto run(std::filesystem::path const& path, TextureKind kind) -> std::error_code;

        /// Builds a job that processes the texture at the given path.
        /// \param path The path to the texture file to be processed.
        /// \param kind The kind of PBR map the texture represents.
        /// \return A job that, when executed, processes the texture.
        [[nodiscard]] auto toJob(std::filesystem::path path, TextureKind kind) -> Job;
    };
} // namespace ac
