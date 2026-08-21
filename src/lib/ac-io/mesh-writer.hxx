//
// Copyright (c) 2026 Jamie Kenyon. All Rights Reserved.
//

#pragma once

#include "ac-io/mesh-cooker.hxx"

namespace ac
{
    /// ---------------------------------------------------------------------------
    /// Only trivially-copyable types may be blitted as raw bytes: no vtable, no
    /// user-defined copy/move, no pointer-with-owning-semantics member. Enforcing
    /// this at the call site (rather than just trusting the caller) turns "silently
    /// wrote garbage" into a compile error the moment someone's struct edit breaks it.
    // ---------------------------------------------------------------------------
    template <typename T>
    concept BinaryBlittable = std::is_trivially_copyable_v<T>;

    /// Identifies the file and lets a loader reject a wrong/old file outright
    /// instead of reinterpreting arbitrary bytes as vertex data.
    struct FileHeader
    {
        std::uint32_t magic{ 0x4853454DU }; ///< ASCII "MESH", read little-endian
        std::uint32_t version{ 1 };         ///< Denotes the file version.
    };
    static_assert(BinaryBlittable<FileHeader>);

    /// Write a mesh to disc.
    /// \param mesh The mesh to write.
    /// \param path The path to write the mesh to.
    /// \return An error code on failure; nothing otherwose.
    [[nodiscard]] auto writeMesh(Mesh const& mesh, std::filesystem::path const path) -> std::error_code;
} // namespace ac
