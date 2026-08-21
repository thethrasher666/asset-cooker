//
// Copyright (c) 2026 Jamie Kenyon. All Rights Reserved.
//

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <filesystem>

#include <ac-io/mesh-cooker.hxx>

namespace ac::test
{
    TEST_CASE("test.ac.mesh-cooker.cook-static-mesh-box-textured")
    {
        auto const path = (std::filesystem::path(AC_TEST_DATA_DIR) / "models" / "box" / "BoxTextured.gltf").lexically_normal();

        auto result = cookStaticMesh(path);
        REQUIRE(result.has_value());

        auto const& mesh = *result;

        CHECK(mesh.vertices.size() == 24);
        CHECK(mesh.indices.size() == 36);
        REQUIRE(mesh.subMeshes.size() == 1);

        auto const& subMesh = mesh.subMeshes[0];
        CHECK(subMesh.firstIndex == 0);
        CHECK(subMesh.indexCount == 36);
        CHECK(subMesh.materialIndex == 0);

        // The mesh-owning node has no transform of its own; its parent's matrix rotates
        // (x, y, z) to (x, z, -y). Probe it with the basis vectors rather than trusting a
        // literal copy of the source JSON's matrix.
        auto const rotatedY = glm::vec3(subMesh.transform * glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
        CHECK(rotatedY.x == Catch::Approx(0.0f).margin(1e-6));
        CHECK(rotatedY.y == Catch::Approx(0.0f).margin(1e-6));
        CHECK(rotatedY.z == Catch::Approx(-1.0f));

        auto const rotatedZ = glm::vec3(subMesh.transform * glm::vec4(0.0f, 0.0f, 1.0f, 1.0f));
        CHECK(rotatedZ.x == Catch::Approx(0.0f).margin(1e-6));
        CHECK(rotatedZ.y == Catch::Approx(1.0f));
        CHECK(rotatedZ.z == Catch::Approx(0.0f).margin(1e-6));

        // Vertex data is no longer baked, so it should match the source accessors verbatim: a
        // half-extent-0.5 cube centred on the origin, first vertex on the +Z face.
        CHECK(mesh.vertices[0].position.x == Catch::Approx(-0.5f));
        CHECK(mesh.vertices[0].position.y == Catch::Approx(-0.5f));
        CHECK(mesh.vertices[0].position.z == Catch::Approx(0.5f));
        CHECK(mesh.vertices[0].normal.x == Catch::Approx(0.0f).margin(1e-6));
        CHECK(mesh.vertices[0].normal.y == Catch::Approx(0.0f).margin(1e-6));
        CHECK(mesh.vertices[0].normal.z == Catch::Approx(1.0f));

        // Bounds still reflect the placed (transformed) extent. The cube is symmetric about the
        // origin, so permuting/flipping its axes leaves the same half-extent-0.5 bounds.
        CHECK(mesh.boundsMin.x == Catch::Approx(-0.5f));
        CHECK(mesh.boundsMin.y == Catch::Approx(-0.5f));
        CHECK(mesh.boundsMin.z == Catch::Approx(-0.5f));
        CHECK(mesh.boundsMax.x == Catch::Approx(0.5f));
        CHECK(mesh.boundsMax.y == Catch::Approx(0.5f));
        CHECK(mesh.boundsMax.z == Catch::Approx(0.5f));

        // NORMAL/POSITION/TEXCOORD_0 are present; TANGENT/COLOR_0/TEXCOORD_1 are not.
        CHECK(mesh.streams == (VertexStream::Normal | VertexStream::Uv0));

        // The primitive has no TANGENT or COLOR_0 attribute, so every vertex should carry the
        // untransformed spec-default tangent/color verbatim.
        for (auto const& vertex : mesh.vertices)
        {
            CHECK(vertex.tangent.x == Catch::Approx(1.0f));
            CHECK(vertex.tangent.y == Catch::Approx(0.0f).margin(1e-6));
            CHECK(vertex.tangent.z == Catch::Approx(0.0f).margin(1e-6));
            CHECK(vertex.tangent.w == Catch::Approx(1.0f));

            CHECK(vertex.color0.x == Catch::Approx(1.0f));
            CHECK(vertex.color0.y == Catch::Approx(1.0f));
            CHECK(vertex.color0.z == Catch::Approx(1.0f));
            CHECK(vertex.color0.w == Catch::Approx(1.0f));
        }
    }

    TEST_CASE("test.ac.mesh-cooker.cook-static-mesh-triangle-strip")
    {
        auto const path = (std::filesystem::path(AC_TEST_DATA_DIR) / "triangle-strip.gltf").lexically_normal();

        auto result = cookStaticMesh(path);
        REQUIRE(result.has_value());

        auto const& mesh = *result;

        REQUIRE(mesh.vertices.size() == 4);

        // Vertex data is no longer baked, so positions should match the source accessor's raw
        // local-space values verbatim; the node's translation lives on subMeshes[0].transform
        // instead.
        struct Expected
        {
            float x, y, z;
        };

        Expected const expectedPositions[] = {
            { 0.0f, 0.0f, 0.0f },
            { 1.0f, 0.0f, 0.0f },
            { 0.0f, 1.0f, 0.0f },
            { 1.0f, 1.0f, 0.0f },
        };

        for (std::size_t i = 0; i < mesh.vertices.size(); ++i)
        {
            auto const& vertex = mesh.vertices[i];
            CHECK(vertex.position.x == Catch::Approx(expectedPositions[i].x));
            CHECK(vertex.position.y == Catch::Approx(expectedPositions[i].y));
            CHECK(vertex.position.z == Catch::Approx(expectedPositions[i].z));

            // No NORMAL/TEXCOORD_0/TEXCOORD_1/TANGENT/COLOR_0 attributes are present: every
            // vertex should carry the untransformed spec-default values verbatim.
            CHECK(vertex.normal.x == Catch::Approx(0.0f).margin(1e-6));
            CHECK(vertex.normal.y == Catch::Approx(0.0f).margin(1e-6));
            CHECK(vertex.normal.z == Catch::Approx(0.0f).margin(1e-6));

            CHECK(vertex.uv0.x == Catch::Approx(0.0f).margin(1e-6));
            CHECK(vertex.uv0.y == Catch::Approx(0.0f).margin(1e-6));

            CHECK(vertex.uv1.x == Catch::Approx(0.0f).margin(1e-6));
            CHECK(vertex.uv1.y == Catch::Approx(0.0f).margin(1e-6));

            CHECK(vertex.tangent.x == Catch::Approx(1.0f));
            CHECK(vertex.tangent.y == Catch::Approx(0.0f).margin(1e-6));
            CHECK(vertex.tangent.z == Catch::Approx(0.0f).margin(1e-6));
            CHECK(vertex.tangent.w == Catch::Approx(1.0f));

            CHECK(vertex.color0.x == Catch::Approx(1.0f));
            CHECK(vertex.color0.y == Catch::Approx(1.0f));
            CHECK(vertex.color0.z == Catch::Approx(1.0f));
            CHECK(vertex.color0.w == Catch::Approx(1.0f));
        }

        CHECK(mesh.streams == VertexStream::None);

        // No indices accessor was supplied, so the cooker synthesizes 0,1,2,3 and then expands
        // TRIANGLE_STRIP into two triangles: (0,1,2) and (2,1,3).
        std::vector<std::uint32_t> const expectedIndices = { 0, 1, 2, 2, 1, 3 };
        CHECK(mesh.indices == expectedIndices);

        REQUIRE(mesh.subMeshes.size() == 1);
        auto const& subMesh = mesh.subMeshes[0];
        CHECK(subMesh.firstIndex == 0);
        CHECK(subMesh.indexCount == 6);
        CHECK(subMesh.materialIndex == -1);

        // The sole node is a pure translation by (10, 20, 30).
        auto const translated = glm::vec3(subMesh.transform * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
        CHECK(translated.x == Catch::Approx(10.0f));
        CHECK(translated.y == Catch::Approx(20.0f));
        CHECK(translated.z == Catch::Approx(30.0f));

        CHECK(subMesh.boundsMin.x == Catch::Approx(10.0f));
        CHECK(subMesh.boundsMin.y == Catch::Approx(20.0f));
        CHECK(subMesh.boundsMin.z == Catch::Approx(30.0f));
        CHECK(subMesh.boundsMax.x == Catch::Approx(11.0f));
        CHECK(subMesh.boundsMax.y == Catch::Approx(21.0f));
        CHECK(subMesh.boundsMax.z == Catch::Approx(30.0f));
    }

    TEST_CASE("test.ac.mesh-cooker.cook-static-mesh-missing-file-fails")
    {
        auto const path = (std::filesystem::path(AC_TEST_DATA_DIR) / "does-not-exist.gltf").lexically_normal();

        auto result = cookStaticMesh(path);
        REQUIRE(!result.has_value());
    }
} // namespace ac::test
