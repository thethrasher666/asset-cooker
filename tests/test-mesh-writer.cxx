//
// Copyright (c) 2026 Jamie Kenyon. All Rights Reserved.
//

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <fstream>
#include <glm/gtc/matrix_transform.hpp>

#include <ac-io/error.hxx>
#include <ac-io/mesh-writer.hxx>
#include <ac-io/temp-dir.hxx>

namespace ac::test
{
    namespace
    {
        class BinaryReader
        {
        public:
            explicit BinaryReader(std::istream& in) : m_in(in)
            {
            }

            template <BinaryBlittable T>
            [[nodiscard]] auto readValue() -> T
            {
                T value{};
                m_in.read(reinterpret_cast<char*>(std::addressof(value)), sizeof(T));
                if (!m_in)
                {
                    throw std::runtime_error("mesh_io: unexpected end of stream reading a value");
                }
                return value;
            }

            template <BinaryBlittable T>
            [[nodiscard]] auto readArray() -> std::vector<T>
            {
                auto const     count = readValue<std::uint64_t>();
                std::vector<T> values(static_cast<std::size_t>(count));
                if (count > 0)
                {
                    m_in.read(reinterpret_cast<char*>(values.data()), static_cast<std::streamsize>(count * sizeof(T)));
                    if (!m_in)
                    {
                        throw std::runtime_error("mesh_io: unexpected end of stream reading an array");
                    }
                }
                return values;
            }

        private:
            std::istream& m_in;
        };

        [[nodiscard]] auto readMesh(BinaryReader& reader) -> Mesh
        {
            auto const header = reader.readValue<FileHeader>();
            if (header.magic != FileHeader{}.magic)
            {
                throw std::runtime_error("mesh_io: bad magic -- not a Mesh file");
            }

            if (header.version != FileHeader{}.version)
            {
                throw std::runtime_error("mesh_io: unsupported Mesh file version " + std::to_string(header.version));
            }

            Mesh mesh;
            mesh.vertices = reader.readArray<Vertex>();
            mesh.indices = reader.readArray<std::uint32_t>();
            mesh.subMeshes = reader.readArray<SubMesh>();
            mesh.streams = reader.readValue<VertexStream>();
            mesh.boundsMin = reader.readValue<glm::vec3>();
            mesh.boundsMax = reader.readValue<glm::vec3>();
            return mesh;
        }

        [[nodiscard]] auto loadMesh(std::filesystem::path const& path) -> Mesh
        {
            std::ifstream in(path, std::ios::binary);
            if (!in)
            {
                throw std::runtime_error("mesh_io: could not open '" + path.generic_string() + "' for reading");
            }
            BinaryReader reader(in);
            return readMesh(reader);
        }

        [[nodiscard]] auto makeSampleMesh() -> Mesh
        {
            Mesh mesh;

            mesh.vertices = {
                Vertex{
                    .position = { -0.5f, -0.5f, 0.5f },
                    .normal = { 0.0f, 0.0f, 1.0f },
                    .uv0 = { 0.0f, 0.0f },
                    .uv1 = { 1.0f, 1.0f },
                    .tangent = { 1.0f, 0.0f, 0.0f, 1.0f },
                    .color0 = { 1.0f, 0.5f, 0.25f, 1.0f },
                },
                Vertex{
                    .position = { 0.5f, 0.5f, -0.5f },
                    .normal = { 0.0f, 1.0f, 0.0f },
                    .uv0 = { 1.0f, 1.0f },
                    .uv1 = { 0.0f, 0.0f },
                    .tangent = { 0.0f, 1.0f, 0.0f, -1.0f },
                    .color0 = { 0.25f, 0.5f, 1.0f, 1.0f },
                },
            };
            mesh.indices = { 0, 1, 0 };
            mesh.subMeshes = {
                SubMesh{
                    .firstIndex = 0,
                    .indexCount = 3,
                    .materialIndex = 2,
                    .transform = glm::translate(glm::mat4(1.0f), glm::vec3(1.0f, 2.0f, 3.0f)),
                    .boundsMin = { -0.5f, -0.5f, -0.5f },
                    .boundsMax = { 0.5f, 0.5f, 0.5f },
                },
            };
            mesh.streams = VertexStream::Normal | VertexStream::Uv0 | VertexStream::Tangent;
            mesh.boundsMin = { -0.5f, -0.5f, -0.5f };
            mesh.boundsMax = { 0.5f, 0.5f, 0.5f };

            return mesh;
        }
    } // namespace

    TEST_CASE("test.ac.mesh-writer.round-trip-preserves-data")
    {
        TempDirectory const tempDir;
        auto const          path = tempDir.child("mesh.bin");
        auto const          source = makeSampleMesh();

        auto const writeError = writeMesh(source, path);
        REQUIRE(!writeError);

        auto const loaded = loadMesh(path);

        REQUIRE(loaded.vertices.size() == source.vertices.size());
        for (std::size_t i = 0; i < source.vertices.size(); ++i)
        {
            CHECK(loaded.vertices[i].position == source.vertices[i].position);
            CHECK(loaded.vertices[i].normal == source.vertices[i].normal);
            CHECK(loaded.vertices[i].uv0 == source.vertices[i].uv0);
            CHECK(loaded.vertices[i].uv1 == source.vertices[i].uv1);
            CHECK(loaded.vertices[i].tangent == source.vertices[i].tangent);
            CHECK(loaded.vertices[i].color0 == source.vertices[i].color0);
        }

        CHECK(loaded.indices == source.indices);

        REQUIRE(loaded.subMeshes.size() == source.subMeshes.size());
        auto const& loadedSubMesh = loaded.subMeshes[0];
        auto const& sourceSubMesh = source.subMeshes[0];
        CHECK(loadedSubMesh.firstIndex == sourceSubMesh.firstIndex);
        CHECK(loadedSubMesh.indexCount == sourceSubMesh.indexCount);
        CHECK(loadedSubMesh.materialIndex == sourceSubMesh.materialIndex);
        CHECK(loadedSubMesh.transform == sourceSubMesh.transform);
        CHECK(loadedSubMesh.boundsMin == sourceSubMesh.boundsMin);
        CHECK(loadedSubMesh.boundsMax == sourceSubMesh.boundsMax);

        CHECK(loaded.streams == source.streams);
        CHECK(loaded.boundsMin == source.boundsMin);
        CHECK(loaded.boundsMax == source.boundsMax);
    }

    TEST_CASE("test.ac.mesh-writer.round-trip-empty-mesh")
    {
        TempDirectory const tempDir;
        auto const          path = tempDir.child("empty.bin");
        Mesh const          source;

        auto const writeError = writeMesh(source, path);
        REQUIRE(!writeError);

        auto const loaded = loadMesh(path);

        CHECK(loaded.vertices.empty());
        CHECK(loaded.indices.empty());
        CHECK(loaded.subMeshes.empty());
        CHECK(loaded.streams == VertexStream::None);
        CHECK(loaded.boundsMin == glm::vec3(0.0f));
        CHECK(loaded.boundsMax == glm::vec3(0.0f));
    }

    TEST_CASE("test.ac.mesh-writer.write-fails-when-output-directory-is-missing")
    {
        TempDirectory const tempDir;
        auto const          path = tempDir.child("does-not-exist") / "mesh.bin";

        auto const writeError = writeMesh(Mesh{}, path);

        REQUIRE(writeError);
        CHECK(writeError == makeErrorCode(ErrorCode::OutputFileCreationFailed));
        CHECK(!std::filesystem::exists(path));
    }
} // namespace ac::test
