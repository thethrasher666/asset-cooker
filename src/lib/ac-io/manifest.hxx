//
// Copyright (c) 2026 Jamie Kenyon. All Rights Reserved.
//

#pragma once

#include <expected>
#include <filesystem>
#include <system_error>
#include <vector>
#include <toml++/toml.h>

namespace ac
{
    /// The manifest is a file containing what files need to be cooked and how to cook them.
    class Manifest final
    {
    public:
        /// Get the paths to the albedo maps.
        /// \return A collection of fully-qualified paths.
        [[nodiscard]] auto albedoMaps() const -> std::vector<std::filesystem::path> const&
        {
            return _albedoMaps;
        }

        /// Get the paths to the ambient occlusion maps.
        /// \return A collection of fully-qualified paths.
        [[nodiscard]] auto ambientOcclusionMaps() const -> std::vector<std::filesystem::path> const&
        {
            return _ambientOcclusionMaps;
        }

        /// Get the paths to the displacement maps.
        /// \return A collection of fully-qualified paths.
        [[nodiscard]] auto displacementMaps() const -> std::vector<std::filesystem::path> const&
        {
            return _displacementMaps;
        }

        /// Get the paths to the emissive maps.
        /// \return A collection of fully-qualified paths.
        [[nodiscard]] auto emissiveMaps() const -> std::vector<std::filesystem::path> const&
        {
            return _emissiveMaps;
        }

        /// Loads a manifest from a TOML file.
        /// \param path The path to the TOML file to load.
        /// \return An error code indicating the success or failure of the loading operation.
        [[nodiscard]] auto load(std::filesystem::path const& path) -> std::error_code;

        /// Get the paths to the metallic maps.
        /// \return A collection of fully-qualified paths.
        [[nodiscard]] auto metallicMaps() const -> std::vector<std::filesystem::path> const&
        {
            return _metallicMaps;
        }

        /// Get the paths to the normal maps.
        /// \return A collection of fully-qualified paths.
        [[nodiscard]] auto normalMaps() const -> std::vector<std::filesystem::path> const&
        {
            return _normalMaps;
        }

        /// Get the paths to the opacity maps.
        /// \return A collection of fully-qualified paths.
        [[nodiscard]] auto opacityMaps() const -> std::vector<std::filesystem::path> const&
        {
            return _opacityMaps;
        }

        /// Get the paths to the roughness maps.
        /// \return A collection of fully-qualified paths.
        [[nodiscard]] auto roughnessMaps() const -> std::vector<std::filesystem::path> const&
        {
            return _roughnessMaps;
        }

        /// Get the paths to the skinned meshes.
        /// \return A collection of fully-qualified paths.
        [[nodiscard]] auto skinnedMeshes() const -> std::vector<std::filesystem::path> const&
        {
            return _skinnedMeshes;
        }

        /// Get the paths to the static meshes.
        /// \return A collection of fully-qualified paths.
        [[nodiscard]] auto staticMeshes() const -> std::vector<std::filesystem::path> const&
        {
            return _staticMeshes;
        }

    private:
        /// Parses the version string from the manifest.
        /// \param version The version string to parse.
        /// \return True if the version string was parsed successfully, false otherwise.
        [[nodiscard]] auto parseVersion(std::string const& version) -> bool;

    private:
        std::vector<std::filesystem::path> _albedoMaps;           ///< The paths to the albedo maps.
        std::vector<std::filesystem::path> _ambientOcclusionMaps; ///< The paths to the ambient occlusion maps.
        std::vector<std::filesystem::path> _displacementMaps;     ///< The paths to the displacement maps.
        std::vector<std::filesystem::path> _emissiveMaps;         ///< The paths to the emissive maps.
        std::vector<std::filesystem::path> _metallicMaps;         ///< The paths to the metallic maps.
        std::vector<std::filesystem::path> _normalMaps;           ///< The paths to the normal maps.
        std::vector<std::filesystem::path> _opacityMaps;          ///< The paths to the opacity maps.
        std::vector<std::filesystem::path> _roughnessMaps;        ///< The paths to the roughness maps.
        std::vector<std::filesystem::path> _skinnedMeshes;        ///< The paths to the skinned meshes.
        std::vector<std::filesystem::path> _staticMeshes;         ///< The paths to the static meshes.
        uint32_t                           _versionMajor{};       ///< The major version of the tool-chain.
        uint32_t                           _versionMinor{};       ///< The minor version of the tool-chain.
    };
} // namespace ac
