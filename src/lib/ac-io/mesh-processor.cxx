//
// Copyright (c) 2026 Jamie Kenyon. All Rights Reserved.
//

#include "ac-io/error.hxx"
#include "ac-io/mesh-processor.hxx"

namespace ac
{
    auto MeshProcessor::run(std::filesystem::path const & path) -> std::error_code
    {
        // Implement the mesh processing logic here.
        return {};
    }

    auto MeshProcessor::toJob(std::filesystem::path path) -> Job
    {
        return Job{
            .name = "Mesh: " + path.filename().string(),
            .work = FunctionWork([this, path = std::move(path)]() -> std::error_code
            {
                return run(path);
            })
        };
    }
} // namespace ac
