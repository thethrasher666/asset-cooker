//
// Copyright (c) 2026 Jamie Kenyon. All Rights Reserved.
//

#pragma once

#include <expected>
#include <filesystem>
#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <tiny_gltf_v3.h>

namespace ac
{
    /// A single static-mesh vertex, laid out for direct upload to a GPU vertex buffer.
    /// Positions/normals/tangents are in the source node's own local space: the node's world
    /// transform is not baked in here, see SubMesh::transform.
    struct Vertex
    {
        glm::vec3 position; ///< Local-space position.
        glm::vec3 normal;   ///< Local-space normal.
        glm::vec2 uv0;      ///< The first texture coordinate channel.
        glm::vec2 uv1;      ///< The second texture coordinate channel.
        glm::vec4 tangent;  ///< xyz is the local-space tangent direction, w is the bitangent handedness (+1/-1).
        glm::vec4 color0;   ///< Vertex color, defaulting to (1,1,1,1) when the source has none.
    };

    /// Flags recording which optional Vertex streams carry real, authored data across an entire
    /// Mesh, as opposed to spec-default fill values. A bit is set only when every contributing
    /// primitive supplied that attribute -- a shared, fixed-layout vertex buffer can't represent
    /// "only some vertices have it".
    enum class VertexStream : std::uint32_t
    {
        None = 0,                                    ///< No vertex streams.
        Normal = 1u << 0,                            ///< Normals.
        Uv0 = 1u << 1,                               ///< UV 0.
        Uv1 = 1u << 2,                               ///< UV 1.
        Tangent = 1u << 3,                           ///< Tangents.
        Color0 = 1u << 4,                            ///< Vertex color.
        All = Normal | Uv0 | Uv1 | Tangent | Color0, ///< All.
    };

    /// A contiguous range of indices sharing one material, ready for a single draw call.
    struct SubMesh
    {
        std::uint32_t firstIndex{};        ///< The first index of this range within Mesh::indices.
        std::uint32_t indexCount{};        ///< The number of indices in this range.
        std::int32_t  materialIndex{ -1 }; ///< The source material index, or -1 if none.
        glm::mat4     transform{ 1.0f };   ///< The source node's accumulated local-to-model transform.
        glm::vec3     boundsMin{};         ///< The minimum corner of this range's placed (transformed) axis-aligned bounds.
        glm::vec3     boundsMax{};         ///< The maximum corner of this range's placed (transformed) axis-aligned bounds.
    };

    /// A cooked static mesh: one shared vertex/index buffer plus a per-material draw table.
    struct Mesh
    {
        std::vector<Vertex>        vertices;                      ///< All vertices from every source primitive, in each one's own local space.
        std::vector<std::uint32_t> indices;                       ///< All indices from every source primitive, offset into the shared vertex buffer.
        std::vector<SubMesh>       subMeshes;                     ///< One entry per source primitive, describing its draw range, material, and transform.
        VertexStream               streams{ VertexStream::None }; ///< Which optional Vertex streams are backed by authored data.
        glm::vec3                  boundsMin{};                   ///< The minimum corner of the whole mesh's placed (transformed) axis-aligned bounds.
        glm::vec3                  boundsMax{};                   ///< The maximum corner of the whole mesh's placed (transformed) axis-aligned bounds.
    };

    [[nodiscard]] constexpr auto operator|(VertexStream const a, VertexStream const b) -> VertexStream
    {
        return static_cast<VertexStream>(static_cast<std::uint32_t>(a) | static_cast<std::uint32_t>(b));
    }

    [[nodiscard]] constexpr auto operator&(VertexStream const a, VertexStream const b) -> VertexStream
    {
        return static_cast<VertexStream>(static_cast<std::uint32_t>(a) & static_cast<std::uint32_t>(b));
    }

    constexpr auto operator|=(VertexStream& a, VertexStream const b) -> VertexStream&
    {
        a = a | b;
        return a;
    }

    constexpr auto operator&=(VertexStream& a, VertexStream const b) -> VertexStream&
    {
        a = a & b;
        return a;
    }

    /// Cooks a static mesh glTF/GLB asset into a POD Mesh, flattening its node hierarchy into a
    /// shared vertex/index buffer. Vertex data stays in each source node's local space; each
    /// SubMesh carries that node's accumulated world transform for the renderer to apply.
    /// \param path Fully-qualified path to a glTF/GLB asset.
    /// \return The cooked mesh, or the error that prevented cooking it.
    [[nodiscard]] auto cookStaticMesh(std::filesystem::path const& path) -> std::expected<Mesh, std::error_code>;
} // namespace ac
