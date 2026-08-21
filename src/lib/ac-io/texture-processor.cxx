//
// Copyright (c) 2026 Jamie Kenyon. All Rights Reserved.
//

#include "ac-io/texture-processor.hxx"
#include "ac-io/error.hxx"

#include <proc-lib/process.hxx>

#include <string>
#include <string_view>
#include <vector>

namespace ac
{
    namespace
    {
        /// The VkFormat (minus the VK_FORMAT_ prefix) and any extra `ktx create` options needed for a texture kind.
        struct KtxFormatOptions
        {
            std::string_view         format;         ///< The --format value to pass to `ktx create`.
            std::vector<std::string> extraArguments; ///< Any additional arguments `ktx create` needs for this kind.
        };

        /// Determines the KTX output format and options for a given texture kind.
        /// Color data (albedo, emissive) is encoded sRGB; everything else is technical/linear data.
        [[nodiscard]] auto formatOptionsFor(TextureKind const kind) -> KtxFormatOptions
        {
            switch (kind)
            {
            case TextureKind::Albedo:
            case TextureKind::Emissive:
                return { .format = "R8G8B8A8_SRGB", .extraArguments = {} };

            case TextureKind::Normal:
                return { .format = "R8G8B8A8_UNORM", .extraArguments = { "--normal-mode", "--normalize" } };

            case TextureKind::AmbientOcclusion:
            case TextureKind::Displacement:
            case TextureKind::Metallic:
            case TextureKind::Opacity:
            case TextureKind::Roughness:
                return { .format = "R8G8B8A8_UNORM", .extraArguments = {} };
            }

            return { .format = "R8G8B8A8_UNORM", .extraArguments = {} };
        }

        /// The path to the bundled `ktx` command-line tool, relative to the current working directory.
        [[nodiscard]] auto ktxExecutablePath() -> std::filesystem::path
        {
#if defined(_WIN32)
            return "bin/ktx.exe";
#else
            return "bin/ktx";
#endif
        }
    } // namespace

    auto TextureProcessor::run(std::filesystem::path const& path, TextureKind kind) -> std::error_code
    {
        auto outputPath = path;
        outputPath.replace_extension(".ktx2");

        auto const options = formatOptionsFor(kind);

        std::vector<std::string> arguments{ "create", "--format", std::string(options.format) };
        arguments.insert(arguments.end(), options.extraArguments.begin(), options.extraArguments.end());
        arguments.push_back("--generate-mipmap");
        arguments.push_back(path.string());
        arguments.push_back(outputPath.string());

        pl::Process process;
        if (auto result = process.start(ktxExecutablePath(), arguments); result)
        {
            return result;
        }

        if (auto const outcome = process.wait(); !outcome.succeeded())
        {
            return makeErrorCode(ErrorCode::AssetProcessingFailed);
        }

        return {};
    }

    auto TextureProcessor::toJob(std::filesystem::path path, TextureKind kind) -> Job
    {
        return Job{ .name = "Texture: " + path.filename().string(), .work = FunctionWork([this, path = std::move(path), kind]() -> std::error_code { return run(path, kind); }) };
    }
} // namespace ac
