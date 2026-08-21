//
// Copyright (c) 2026 Jamie Kenyon. All Rights Reserved.
//

#pragma once

#include <atomic>
#include <filesystem>

namespace ac
{
    /// A process-wide object that owns one randomly named temporary directory for the lifetime of the object.
    class TempDirectory
    {
    public:
        /// Constructor.
        TempDirectory();

        /// Destructor.
        ~TempDirectory();

        /// Acccessor.
        /// \return A pointer to the instance if there is one, nothing otherwise.
        static auto instance() -> TempDirectory*;

        /// Not copyable.
        TempDirectory(TempDirectory const&) = delete;

        /// Not copyable.
        TempDirectory& operator=(TempDirectory const&) = delete;

        /// Not moveable.
        TempDirectory(TempDirectory&&) = delete;

        /// Not moveable.
        TempDirectory& operator=(TempDirectory&&) = delete;

        /// Get the directory itself.
        /// \return A valid path.
        [[nodiscard]] auto path() const noexcept -> std::filesystem::path const&
        {
            return _dir;
        }

        /// Build a path for a file/subdirectory to create inside the temp directory.
        /// \param name The file/subdirectory.
        /// \return A valid path.
        [[nodiscard]] auto child(std::filesystem::path const& name) const -> std::filesystem::path
        {
            return _dir / name;
        }

        /// Recursively removes the directory.
        void cleanup() noexcept;

    private:
        std::filesystem::path _dir;
        std::atomic<bool>     _cleaned{ false };
    };
} // namespace ac
