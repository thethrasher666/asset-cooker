//
// Copyright (c) 2026 Jamie Kenyon. All Rights Reserved.
//

#include "ac-io/error.hxx"
#include "ac-io/job.hxx"
#include "ac-io/mesh-cooker.hxx"
#include "ac-io/temp-dir.hxx"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <algorithm>
#include <cmath>
#include <cstring>
#include <limits>
#include <numeric>

namespace ac
{
    namespace
    {
        // glTF primitive modes, per the glTF 2.0 specification (fixed values, same as the
        // underlying GL enum). Only these three describe a renderable triangle surface.
        constexpr std::int32_t kModeTriangles = 4;
        constexpr std::int32_t kModeTriangleStrip = 5;
        constexpr std::int32_t kModeTriangleFan = 6;

        // Node transforms are composed/accumulated in double precision (matching glTF's own
        // node.matrix/translation/rotation/scale storage) and only narrowed to float once, at
        // the SubMesh boundary.
        using Mat4 = glm::dmat4;

        [[nodiscard]] auto composeTRS(tg3_node const& node) -> Mat4
        {
            auto const translation = glm::make_vec3(node.translation);
            auto const rotation = glm::make_quat(node.rotation); // glTF and glm both store quats as (x, y, z, w).
            auto const scale = glm::make_vec3(node.scale);

            return glm::translate(Mat4(1.0), translation) * glm::mat4_cast(rotation) * glm::scale(Mat4(1.0), scale);
        }

        [[nodiscard]] auto localMatrix(tg3_node const& node) -> Mat4
        {
            if (node.has_matrix)
            {
                return glm::make_mat4(node.matrix);
            }

            return composeTRS(node);
        }

        [[nodiscard]] auto transformPoint(Mat4 const& m, glm::vec3 const& p) -> glm::vec3
        {
            return glm::vec3(m * glm::dvec4(glm::dvec3(p), 1.0));
        }

        /// A validated view onto one accessor's element data.
        struct AccessorView
        {
            tg3_accessor const* accessor{};
            std::uint8_t const* base{};
            std::int32_t        stride{};
        };

        [[nodiscard]] auto locateAccessorData(tg3_model const& model, std::int32_t const accessorIndex) -> std::expected<AccessorView, std::error_code>
        {
            if (accessorIndex < 0 || static_cast<std::uint32_t>(accessorIndex) >= model.accessors_count)
            {
                return std::unexpected(makeErrorCode(ErrorCode::MeshInvalidAccessorData));
            }

            auto const& accessor = model.accessors[accessorIndex];

            if (accessor.buffer_view < 0 || static_cast<std::uint32_t>(accessor.buffer_view) >= model.buffer_views_count)
            {
                return std::unexpected(makeErrorCode(ErrorCode::MeshInvalidAccessorData));
            }

            auto const& bufferView = model.buffer_views[accessor.buffer_view];

            if (bufferView.buffer < 0 || static_cast<std::uint32_t>(bufferView.buffer) >= model.buffers_count)
            {
                return std::unexpected(makeErrorCode(ErrorCode::MeshInvalidAccessorData));
            }

            auto const& buffer = model.buffers[bufferView.buffer];

            auto const stride = tg3_accessor_byte_stride(&accessor, &bufferView);
            auto const elementSize = static_cast<std::uint64_t>(tg3_component_size(accessor.component_type)) * static_cast<std::uint64_t>(tg3_num_components(accessor.type));

            if (stride <= 0 || elementSize == 0 || accessor.count == 0)
            {
                return std::unexpected(makeErrorCode(ErrorCode::MeshInvalidAccessorData));
            }

            auto const byteStart = bufferView.byte_offset + accessor.byte_offset;
            auto const requiredBytes = byteStart + static_cast<std::uint64_t>(stride) * (accessor.count - 1) + elementSize;

            if (requiredBytes > buffer.data.count)
            {
                return std::unexpected(makeErrorCode(ErrorCode::MeshInvalidAccessorData));
            }

            return AccessorView{ .accessor = &accessor, .base = buffer.data.data + byteStart, .stride = stride };
        }

        /// Decodes one element's components into floats, applying the glTF normalization rules.
        void readComponents(tg3_accessor const& accessor, std::uint8_t const* element, float* out, std::int32_t const componentCount)
        {
            for (std::int32_t c = 0; c < componentCount; ++c)
            {
                switch (accessor.component_type)
                {
                case TG3_COMPONENT_TYPE_FLOAT:
                {
                    float value;
                    std::memcpy(&value, element + static_cast<std::size_t>(c) * sizeof(float), sizeof(float));
                    out[c] = value;
                    break;
                }

                case TG3_COMPONENT_TYPE_UNSIGNED_BYTE:
                {
                    auto const raw = element[c];
                    out[c] = accessor.normalized ? static_cast<float>(raw) / 255.0f : static_cast<float>(raw);
                    break;
                }

                case TG3_COMPONENT_TYPE_BYTE:
                {
                    auto const raw = static_cast<std::int8_t>(element[c]);
                    out[c] = accessor.normalized ? std::max(static_cast<float>(raw) / 127.0f, -1.0f) : static_cast<float>(raw);
                    break;
                }

                case TG3_COMPONENT_TYPE_UNSIGNED_SHORT:
                {
                    std::uint16_t value;
                    std::memcpy(&value, element + static_cast<std::size_t>(c) * sizeof(value), sizeof(value));
                    out[c] = accessor.normalized ? static_cast<float>(value) / 65535.0f : static_cast<float>(value);
                    break;
                }

                case TG3_COMPONENT_TYPE_SHORT:
                {
                    std::int16_t value;
                    std::memcpy(&value, element + static_cast<std::size_t>(c) * sizeof(value), sizeof(value));
                    out[c] = accessor.normalized ? std::max(static_cast<float>(value) / 32767.0f, -1.0f) : static_cast<float>(value);
                    break;
                }

                default:
                    out[c] = 0.0f;
                    break;
                }
            }
        }

        [[nodiscard]] auto findAttribute(tg3_primitive const& primitive, char const* name) -> std::int32_t
        {
            for (std::uint32_t i = 0; i < primitive.attributes_count; ++i)
            {
                if (tg3_str_equals_cstr(primitive.attributes[i].key, name))
                {
                    return primitive.attributes[i].value;
                }
            }

            return -1;
        }

        [[nodiscard]] auto readVec2Array(tg3_model const& model, std::int32_t const accessorIndex) -> std::expected<std::vector<glm::vec2>, std::error_code>
        {
            auto view = locateAccessorData(model, accessorIndex);
            if (!view)
            {
                return std::unexpected(view.error());
            }

            if (tg3_num_components(view->accessor->type) != 2)
            {
                return std::unexpected(makeErrorCode(ErrorCode::MeshInvalidAccessorData));
            }

            std::vector<glm::vec2> result;
            result.reserve(view->accessor->count);

            for (std::uint64_t i = 0; i < view->accessor->count; ++i)
            {
                float components[2];
                readComponents(*view->accessor, view->base + i * static_cast<std::uint64_t>(view->stride), components, 2);
                result.push_back(glm::vec2(components[0], components[1]));
            }

            return result;
        }

        [[nodiscard]] auto readVec3Array(tg3_model const& model, std::int32_t const accessorIndex) -> std::expected<std::vector<glm::vec3>, std::error_code>
        {
            auto view = locateAccessorData(model, accessorIndex);
            if (!view)
            {
                return std::unexpected(view.error());
            }

            if (tg3_num_components(view->accessor->type) != 3)
            {
                return std::unexpected(makeErrorCode(ErrorCode::MeshInvalidAccessorData));
            }

            std::vector<glm::vec3> result;
            result.reserve(view->accessor->count);

            for (std::uint64_t i = 0; i < view->accessor->count; ++i)
            {
                float components[3];
                readComponents(*view->accessor, view->base + i * static_cast<std::uint64_t>(view->stride), components, 3);
                result.push_back(glm::vec3(components[0], components[1], components[2]));
            }

            return result;
        }

        [[nodiscard]] auto readVec4Array(tg3_model const& model, std::int32_t const accessorIndex) -> std::expected<std::vector<glm::vec4>, std::error_code>
        {
            auto view = locateAccessorData(model, accessorIndex);
            if (!view)
            {
                return std::unexpected(view.error());
            }

            if (tg3_num_components(view->accessor->type) != 4)
            {
                return std::unexpected(makeErrorCode(ErrorCode::MeshInvalidAccessorData));
            }

            std::vector<glm::vec4> result;
            result.reserve(view->accessor->count);

            for (std::uint64_t i = 0; i < view->accessor->count; ++i)
            {
                float components[4];
                readComponents(*view->accessor, view->base + i * static_cast<std::uint64_t>(view->stride), components, 4);
                result.push_back(glm::vec4(components[0], components[1], components[2], components[3]));
            }

            return result;
        }

        /// COLOR_0 may be VEC3 (alpha defaults to 1) or VEC4, per the glTF spec.
        [[nodiscard]] auto readColorArray(tg3_model const& model, std::int32_t const accessorIndex) -> std::expected<std::vector<glm::vec4>, std::error_code>
        {
            auto view = locateAccessorData(model, accessorIndex);
            if (!view)
            {
                return std::unexpected(view.error());
            }

            auto const componentCount = tg3_num_components(view->accessor->type);

            if (componentCount != 3 && componentCount != 4)
            {
                return std::unexpected(makeErrorCode(ErrorCode::MeshInvalidAccessorData));
            }

            std::vector<glm::vec4> result;
            result.reserve(view->accessor->count);

            for (std::uint64_t i = 0; i < view->accessor->count; ++i)
            {
                float components[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
                readComponents(*view->accessor, view->base + i * static_cast<std::uint64_t>(view->stride), components, componentCount);
                result.push_back(glm::vec4(components[0], components[1], components[2], components[3]));
            }

            return result;
        }

        [[nodiscard]] auto readIndices(tg3_model const& model, std::int32_t const accessorIndex) -> std::expected<std::vector<std::uint32_t>, std::error_code>
        {
            auto view = locateAccessorData(model, accessorIndex);
            if (!view)
            {
                return std::unexpected(view.error());
            }

            std::vector<std::uint32_t> result;
            result.reserve(view->accessor->count);

            for (std::uint64_t i = 0; i < view->accessor->count; ++i)
            {
                auto const* element = view->base + i * static_cast<std::uint64_t>(view->stride);

                switch (view->accessor->component_type)
                {
                case TG3_COMPONENT_TYPE_UNSIGNED_BYTE:
                    result.push_back(*element);
                    break;

                case TG3_COMPONENT_TYPE_UNSIGNED_SHORT:
                {
                    std::uint16_t value;
                    std::memcpy(&value, element, sizeof(value));
                    result.push_back(value);
                    break;
                }

                case TG3_COMPONENT_TYPE_UNSIGNED_INT:
                {
                    std::uint32_t value;
                    std::memcpy(&value, element, sizeof(value));
                    result.push_back(value);
                    break;
                }

                default:
                    return std::unexpected(makeErrorCode(ErrorCode::MeshInvalidAccessorData));
                }
            }

            return result;
        }

        /// Expands strip/fan topologies to a plain triangle list so every SubMesh can be drawn
        /// with a single, uniform index range.
        [[nodiscard]] auto expandToTriangleList(std::vector<std::uint32_t> const& source, std::int32_t const mode) -> std::expected<std::vector<std::uint32_t>, std::error_code>
        {
            if (mode == -1 || mode == kModeTriangles)
            {
                if (source.size() % 3 != 0)
                {
                    return std::unexpected(makeErrorCode(ErrorCode::MeshInvalidAccessorData));
                }

                return source;
            }

            if (mode == kModeTriangleStrip)
            {
                std::vector<std::uint32_t> result;

                if (source.size() < 3)
                {
                    return result;
                }

                result.reserve((source.size() - 2) * 3);

                for (std::size_t i = 0; i + 2 < source.size(); ++i)
                {
                    if (i % 2 == 0)
                    {
                        result.insert(result.end(), { source[i], source[i + 1], source[i + 2] });
                    }
                    else
                    {
                        result.insert(result.end(), { source[i + 1], source[i], source[i + 2] });
                    }
                }

                return result;
            }

            if (mode == kModeTriangleFan)
            {
                std::vector<std::uint32_t> result;

                if (source.size() < 3)
                {
                    return result;
                }

                result.reserve((source.size() - 2) * 3);

                for (std::size_t i = 1; i + 1 < source.size(); ++i)
                {
                    result.insert(result.end(), { source[0], source[i], source[i + 1] });
                }

                return result;
            }

            return std::unexpected(makeErrorCode(ErrorCode::MeshUnsupportedPrimitiveMode));
        }

        [[nodiscard]] auto appendPrimitive(tg3_model const& model, tg3_primitive const& primitive, Mat4 const& worldMatrix, Mesh& mesh) -> std::error_code
        {
            auto const positionAccessor = findAttribute(primitive, "POSITION");

            if (positionAccessor < 0)
            {
                return makeErrorCode(ErrorCode::MeshMissingPositionAttribute);
            }

            auto positions = readVec3Array(model, positionAccessor);
            if (!positions)
            {
                return positions.error();
            }

            auto const vertexCount = positions->size();

            auto presentStreams = VertexStream::None;

            std::vector<glm::vec3> normals(vertexCount);
            if (auto const idx = findAttribute(primitive, "NORMAL"); idx >= 0)
            {
                auto result = readVec3Array(model, idx);
                if (!result)
                {
                    return result.error();
                }
                normals = std::move(*result);
                presentStreams |= VertexStream::Normal;
            }

            std::vector<glm::vec2> uvs(vertexCount);
            if (auto const idx = findAttribute(primitive, "TEXCOORD_0"); idx >= 0)
            {
                auto result = readVec2Array(model, idx);
                if (!result)
                {
                    return result.error();
                }
                uvs = std::move(*result);
                presentStreams |= VertexStream::Uv0;
            }

            std::vector<glm::vec2> uv1s(vertexCount);
            if (auto const idx = findAttribute(primitive, "TEXCOORD_1"); idx >= 0)
            {
                auto result = readVec2Array(model, idx);
                if (!result)
                {
                    return result.error();
                }
                uv1s = std::move(*result);
                presentStreams |= VertexStream::Uv1;
            }

            std::vector<glm::vec4> tangents(vertexCount, glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
            if (auto const idx = findAttribute(primitive, "TANGENT"); idx >= 0)
            {
                auto result = readVec4Array(model, idx);
                if (!result)
                {
                    return result.error();
                }
                tangents = std::move(*result);
                presentStreams |= VertexStream::Tangent;
            }

            std::vector<glm::vec4> colors(vertexCount, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
            if (auto const idx = findAttribute(primitive, "COLOR_0"); idx >= 0)
            {
                auto result = readColorArray(model, idx);
                if (!result)
                {
                    return result.error();
                }
                colors = std::move(*result);
                presentStreams |= VertexStream::Color0;
            }

            if (normals.size() != vertexCount || uvs.size() != vertexCount || uv1s.size() != vertexCount || tangents.size() != vertexCount || colors.size() != vertexCount)
            {
                return makeErrorCode(ErrorCode::MeshInvalidAccessorData);
            }

            mesh.streams &= presentStreams;

            auto const baseVertex = static_cast<std::uint32_t>(mesh.vertices.size());

            mesh.vertices.reserve(mesh.vertices.size() + vertexCount);

            auto subBoundsMin = glm::vec3(std::numeric_limits<float>::max());
            auto subBoundsMax = glm::vec3(std::numeric_limits<float>::lowest());

            for (std::size_t i = 0; i < vertexCount; ++i)
            {
                mesh.vertices.push_back(Vertex{
                .position = (*positions)[i],
                .normal = normals[i],
                .uv0 = uvs[i],
                .uv1 = uv1s[i],
                .tangent = tangents[i],
                .color0 = colors[i],
                });

                // Bounds still need to reflect the *placed* (transformed) extent for culling, even
                // though the stored vertex data itself stays in local space.
                auto const placedPosition = transformPoint(worldMatrix, (*positions)[i]);

                subBoundsMin = glm::min(subBoundsMin, placedPosition);
                subBoundsMax = glm::max(subBoundsMax, placedPosition);
            }

            std::vector<std::uint32_t> rawIndices;

            if (primitive.indices >= 0)
            {
                auto result = readIndices(model, primitive.indices);
                if (!result)
                {
                    return result.error();
                }
                rawIndices = std::move(*result);
            }
            else
            {
                rawIndices.resize(vertexCount);
                std::iota(rawIndices.begin(), rawIndices.end(), 0u);
            }

            auto triangleList = expandToTriangleList(rawIndices, primitive.mode);
            if (!triangleList)
            {
                return triangleList.error();
            }

            auto const firstIndex = static_cast<std::uint32_t>(mesh.indices.size());

            mesh.indices.reserve(mesh.indices.size() + triangleList->size());
            for (auto const localIndex : *triangleList)
            {
                mesh.indices.push_back(baseVertex + localIndex);
            }

            mesh.subMeshes.push_back(SubMesh{
            .firstIndex = firstIndex,
            .indexCount = static_cast<std::uint32_t>(triangleList->size()),
            .materialIndex = primitive.material,
            .transform = glm::mat4(worldMatrix),
            .boundsMin = subBoundsMin,
            .boundsMax = subBoundsMax,
            });

            mesh.boundsMin = glm::min(mesh.boundsMin, subBoundsMin);
            mesh.boundsMax = glm::max(mesh.boundsMax, subBoundsMax);

            return {};
        }

        [[nodiscard]] auto walkNode(tg3_model const& model, std::int32_t const nodeIndex, Mat4 const& parentWorld, Mesh& mesh) -> std::error_code
        {
            if (nodeIndex < 0 || static_cast<std::uint32_t>(nodeIndex) >= model.nodes_count)
            {
                return makeErrorCode(ErrorCode::MeshInvalidAccessorData);
            }

            auto const& node = model.nodes[nodeIndex];
            auto const  world = parentWorld * localMatrix(node);

            if (node.mesh >= 0)
            {
                if (static_cast<std::uint32_t>(node.mesh) >= model.meshes_count)
                {
                    return makeErrorCode(ErrorCode::MeshInvalidAccessorData);
                }

                auto const& sourceMesh = model.meshes[node.mesh];

                for (std::uint32_t p = 0; p < sourceMesh.primitives_count; ++p)
                {
                    if (auto const error = appendPrimitive(model, sourceMesh.primitives[p], world, mesh); error)
                    {
                        return error;
                    }
                }
            }

            for (std::uint32_t c = 0; c < node.children_count; ++c)
            {
                if (auto const error = walkNode(model, node.children[c], world, mesh); error)
                {
                    return error;
                }
            }

            return {};
        }

        [[nodiscard]] auto buildStaticMesh(tg3_model const& model) -> std::expected<Mesh, std::error_code>
        {
            if (model.meshes_count == 0)
            {
                return std::unexpected(makeErrorCode(ErrorCode::MeshDataEmpty));
            }

            Mesh mesh;
            mesh.streams = VertexStream::All;
            mesh.boundsMin = glm::vec3(std::numeric_limits<float>::max());
            mesh.boundsMax = glm::vec3(std::numeric_limits<float>::lowest());

            Mat4 const identity{ 1.0 };

            if (model.scenes_count > 0)
            {
                auto const sceneIndex =
                (model.default_scene >= 0 && static_cast<std::uint32_t>(model.default_scene) < model.scenes_count) ? static_cast<std::uint32_t>(model.default_scene) : 0u;
                auto const& scene = model.scenes[sceneIndex];

                for (std::uint32_t i = 0; i < scene.nodes_count; ++i)
                {
                    if (auto const error = walkNode(model, scene.nodes[i], identity, mesh); error)
                    {
                        return std::unexpected(error);
                    }
                }
            }
            else
            {
                // No scene information at all: fall back to treating every node that isn't
                // referenced as someone else's child as a root, so no node is ever walked twice.
                std::vector<bool> isChild(model.nodes_count, false);

                for (std::uint32_t i = 0; i < model.nodes_count; ++i)
                {
                    auto const& node = model.nodes[i];

                    for (std::uint32_t c = 0; c < node.children_count; ++c)
                    {
                        if (node.children[c] >= 0 && static_cast<std::uint32_t>(node.children[c]) < model.nodes_count)
                        {
                            isChild[static_cast<std::size_t>(node.children[c])] = true;
                        }
                    }
                }

                for (std::uint32_t i = 0; i < model.nodes_count; ++i)
                {
                    if (isChild[i])
                    {
                        continue;
                    }

                    if (auto const error = walkNode(model, static_cast<std::int32_t>(i), identity, mesh); error)
                    {
                        return std::unexpected(error);
                    }
                }
            }

            if (mesh.vertices.empty())
            {
                return std::unexpected(makeErrorCode(ErrorCode::MeshDataEmpty));
            }

            return mesh;
        }
    } // namespace

    auto cookStaticMesh(std::filesystem::path const& path) -> std::expected<Mesh, std::error_code>
    {
        tinygltf3::ErrorStack errorStack;
        tinygltf3::Model      model;

        if (TG3_OK != tinygltf3::parse_file(model, errorStack, path.string().data()))
        {
            return std::unexpected(makeErrorCode(ErrorCode::GltfParseFailed));
        }

        return buildStaticMesh(*model.get());
    }
} // namespace ac
