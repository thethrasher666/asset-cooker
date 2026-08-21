//
// Copyright (c) 2026 Jamie Kenyon. All Rights Reserved.
//

#include "ac-io/mesh-writer.hxx"
#include "ac-io/error.hxx"

#include <concepts>
#include <fstream>
#include <span>

namespace ac
{
    namespace
    {
        class BinaryWriter
        {
        public:
            explicit BinaryWriter(std::ostream& out) : m_out(out)
            {
            }

            template <BinaryBlittable T>
            void writeValue(const T& value)
            {
                m_out.write(reinterpret_cast<const char*>(std::addressof(value)), sizeof(T));
            }

            template <BinaryBlittable T>
            void writeArray(std::span<const T> values)
            {
                writeValue(static_cast<std::uint64_t>(values.size()));
                if (!values.empty())
                {
                    m_out.write(reinterpret_cast<const char*>(values.data()), static_cast<std::streamsize>(values.size_bytes()));
                }
            }

            template <BinaryBlittable T>
            void writeArray(const std::vector<T>& values)
            {
                writeArray(std::span<const T>(values));
            }

        private:
            std::ostream& m_out;
        };
    } // namespace

    auto writeMesh(Mesh const& mesh, std::filesystem::path const path) -> std::error_code
    {
        std::ofstream out(path, std::ios::binary);
        if (!out)
        {
            return makeErrorCode(ErrorCode::OutputFileCreationFailed);
        }

        BinaryWriter writer(out);

        writer.writeValue(FileHeader{});
        writer.writeArray(mesh.vertices);
        writer.writeArray(mesh.indices);
        writer.writeArray(mesh.subMeshes);
        writer.writeValue(mesh.streams);
        writer.writeValue(mesh.boundsMin);
        writer.writeValue(mesh.boundsMax);

        return {};
    }
} // namespace ac
