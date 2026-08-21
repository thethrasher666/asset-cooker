//
// Copyright (c) 2026 Jamie Kenyon. All Rights Reserved.
//

#include "ac-io/temp-dir.hxx"

#include <format>
#include <print>
#include <random>

namespace ac
{
    namespace
    {
        static TempDirectory* s_instance{};
    }

    TempDirectory::TempDirectory()
    {
        std::filesystem::path const base = std::filesystem::temp_directory_path();

        std::random_device                           rd;
        std::mt19937_64                              gen(rd());
        std::uniform_int_distribution<std::uint64_t> dist;

        constexpr int maxAttempts = 8;
        for (int attempt = 0; attempt < maxAttempts; ++attempt)
        {
            const auto            name = std::format("asset-cooker-{:x}", dist(gen));
            std::filesystem::path candidate = base / name;

            std::error_code ec;
            if (std::filesystem::create_directory(candidate, ec))
            {
                _dir = candidate;
                std::filesystem::permissions(_dir, std::filesystem::perms::owner_all, std::filesystem::perm_options::replace, ec);
                break;
            }
        }

        s_instance = this;
    }

    TempDirectory::~TempDirectory()
    {
        cleanup();
        s_instance = nullptr;
    }

    auto TempDirectory::instance() -> TempDirectory*
    {
        return s_instance;
    }

    void TempDirectory::cleanup() noexcept
    {
        if (!_cleaned.exchange(true))
        {
            std::error_code ec;
            std::filesystem::remove_all(_dir, ec);
            if (ec)
            {
                std::println(stderr, "TempDirectory: failed to remove {}: {}", _dir.string(), ec.message());
            }
        }
    }
} // namespace ac
