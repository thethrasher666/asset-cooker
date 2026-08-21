//
// Copyright (c) 2026 Jamie Kenyon. All Rights Reserved.
//

#include <ac-io/manifest.hxx>
#include <ac-io/processor.hxx>

#include <print>
#include <string>
#include <string_view>
#include <vector>

namespace ac
{
    void printUsage(std::string_view programName)
    {
        std::println(stdout, "Usage: {} <manifest_file>", programName);
    }

    [[nodiscard]] auto entryPoint(std::vector<std::string> const& args) -> bool
    {
        if (args.size() != 2)
        {
            printUsage(args[0]);
            return false;
        }

        ac::Manifest manifest;
        if (auto result = manifest.load(std::filesystem::path(args[1])); result)
        {
            std::println(stderr, "Failed to load manifest: {}", result.message());
            return false;
        }

        ac::Processor processor;
        if (auto result = processor.run(manifest); result)
        {
            std::println(stderr, "Failed to process manifest: {}", result.message());
            return false;
        }
        return true;
    }
} // namespace ac

auto main(int32_t argc, char const* argv[]) -> int32_t
{
    std::vector<std::string> args(argv, argv + argc);
    return ac::entryPoint(args) ? EXIT_SUCCESS : EXIT_FAILURE;
}
