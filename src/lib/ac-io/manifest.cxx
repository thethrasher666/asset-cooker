//
// Copyright (c) 2026 Jamie Kenyon. All Rights Reserved.
//

#include "ac-io/manifest.hxx"
#include "ac-io/error.hxx"
#include "ac-io/version.hxx"

#include <charconv>
#include <regex>
#include <string>
#include <string_view>

namespace ac
{
    namespace
    {
        auto parseComponent(std::string_view const text, std::uint32_t& value) -> bool
        {
            auto const [end, error] = std::from_chars(text.data(), text.data() + text.size(), value);
            return error == std::errc{} && end == text.data() + text.size();
        }

        /// Resolves a manifest entry to a fully-qualified path.
        /// \param manifestDirectory The directory containing the manifest file.
        /// \param entry The path as it appears in the manifest, either absolute or relative to the
        /// \return The fully-qualified, normalized path.
        [[nodiscard]] auto resolvePath(std::filesystem::path const& manifestDirectory, std::string_view const entry) -> std::filesystem::path
        {
            std::filesystem::path const path{ entry };

            return path.is_absolute() ? path : std::filesystem::weakly_canonical(manifestDirectory / path);
        }

        /// Loads the file entries listed under a section of the
        /// \param table The root TOML table.
        /// \param section The name of the section to load entries from.
        /// \param manifestDir The directory containing the manifest file.
        /// \return The fully-qualified paths of the entries in the section.
        [[nodiscard]] auto loadEntries(toml::table const& table, std::string_view const section, std::filesystem::path const& manifestDir) -> std::vector<std::filesystem::path>
        {
            std::vector<std::filesystem::path> entries;

            if (auto const* files = table[section]["files"].as_array())
            {
                for (auto const& node : *files)
                {
                    if (auto const value = node.value<std::string>())
                    {
                        entries.push_back(resolvePath(manifestDir, *value));
                    }
                }
            }

            return entries;
        }
    } // namespace

    auto Manifest::load(std::filesystem::path const& path) -> std::error_code
    {
        auto result = toml::parse_file(path.c_str());

        if (!result)
        {
            return makeErrorCode(ErrorCode::ManifestParseFailed);
        }

        auto const table = std::move(result).table();
        auto const manifestDirectory = path.parent_path();

        if (auto const version = table["version"].value<std::string>())
        {
            if (!parseVersion(*version))
            {
                return makeErrorCode(ErrorCode::InvalidVersionIdentifier);
            }

            if (_versionMajor < ac::version::major() || _versionMinor < ac::version::minor())
            {
                return makeErrorCode(ErrorCode::ToolchainVersionTooOld);
            }
        }

        _albedoMaps = loadEntries(table, "albedo", manifestDirectory);
        _ambientOcclusionMaps = loadEntries(table, "ambient_occlusion", manifestDirectory);
        _displacementMaps = loadEntries(table, "displacement", manifestDirectory);
        _emissiveMaps = loadEntries(table, "emissive", manifestDirectory);
        _metallicMaps = loadEntries(table, "metallic", manifestDirectory);
        _normalMaps = loadEntries(table, "normal", manifestDirectory);
        _opacityMaps = loadEntries(table, "opacity", manifestDirectory);
        _roughnessMaps = loadEntries(table, "roughness", manifestDirectory);
        _skinnedMeshes = loadEntries(table, "skinned_mesh", manifestDirectory);
        _staticMeshes = loadEntries(table, "static_mesh", manifestDirectory);

        return {};
    }

    auto Manifest::parseVersion(std::string const& version) -> bool
    {
        static const std::regex version_regex(R"(^(\d+)\.(\d+)$)");
        std::smatch             matches;

        if (!std::regex_match(version, matches, version_regex))
        {
            return false;
        }

        auto const major = matches[1].str();
        auto const minor = matches[2].str();

        // The regex guarantees digits, so from_chars can only fail on overflow.
        return parseComponent(major, _versionMajor) && parseComponent(minor, _versionMinor);
    }
} // namespace ac
