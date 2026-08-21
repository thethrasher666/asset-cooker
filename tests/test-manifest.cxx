//
// Copyright (c) 2026 Jamie Kenyon. All Rights Reserved.
//

#include <catch2/catch_test_macros.hpp>

#include <filesystem>

#include <ac-io/manifest.hxx>

namespace ac::test
{
    TEST_CASE("test.ac.manifest")
    {
        auto const manifestPath = (std::filesystem::path(AC_TEST_DATA_DIR) / "manifest.toml").lexically_normal();
        Manifest   manifest;

        auto result = manifest.load(manifestPath);
        REQUIRE(!result);

        CHECK(manifest.albedoMaps().empty());
        CHECK(manifest.ambientOcclusionMaps().empty());
        CHECK(manifest.displacementMaps().empty());
        CHECK(manifest.emissiveMaps().empty());
        CHECK(manifest.metallicMaps().empty());
        CHECK(manifest.normalMaps().empty());
        CHECK(manifest.opacityMaps().empty());
        CHECK(manifest.roughnessMaps().empty());
        CHECK(manifest.skinnedMeshes().empty());
        CHECK(manifest.staticMeshes().empty());
    }
} // namespace ac::test
