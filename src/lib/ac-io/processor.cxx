//
// Copyright (c) 2026 Jamie Kenyon. All Rights Reserved.
//

#include "ac-io/processor.hxx"
#include "ac-io/error.hxx"
#include "ac-io/job.hxx"
#include "ac-io/mesh-cooker.hxx"
#include "ac-io/mesh-writer.hxx"
#include "ac-io/temp-dir.hxx"

#include <algorithm>

namespace ac
{
    namespace
    {
        [[nodiscard]] auto processStaticMeshFile(std::filesystem::path const& input, std::filesystem::path const& output) -> std::error_code
        {
            auto meshResult = cookStaticMesh(input);

            if (!meshResult)
            {
                return meshResult.error();
            }

            return writeMesh(*meshResult, output);
        }

        [[nodiscard]] auto createStaticMeshJob(std::filesystem::path path) -> Job
        {
            auto fileName = path.filename();
            Job  job;

            job.name = "Static Mesh: " + fileName.generic_string();
            job.input = std::move(path);
            job.output = TempDirectory::instance()->child(fileName.replace_extension(".bin"));
            job.work = [](std::filesystem::path const& input, std::filesystem::path const& output) -> JobOutcome { return { .error = processStaticMeshFile(input, output) }; };

            return job;
        }

        [[nodiscard]] auto processSkinnedMeshFile(std::filesystem::path const& input, std::filesystem::path const& output) -> std::error_code
        {
            auto meshResult = cookStaticMesh(input);

            if (!meshResult)
            {
                return meshResult.error();
            }

            if (auto result = writeMesh(*meshResult, output); result)
            {
                return result;
            }

            // TODO: skinned mesh cooking (joints/weights) is not yet implemented.
            return {};
        }

        [[nodiscard]] auto createSkinnedMeshJob(std::filesystem::path path) -> Job
        {
            auto fileName = path.filename();
            Job  job;

            job.name = "Skinned Mesh: " + fileName.generic_string();
            job.input = std::move(path);
            job.output = TempDirectory::instance()->child(fileName.replace_extension(".bin"));
            job.work = [](std::filesystem::path const& input, std::filesystem::path const& output) -> JobOutcome { return { .error = processSkinnedMeshFile(input, output) }; };

            return job;
        }
    } // namespace

    auto Processor::run(Manifest const& man) -> std::error_code
    {
        JobExecutor executor;
        auto        tempDir = std::make_unique<TempDirectory>();

        std::for_each(man.skinnedMeshes().begin(), man.skinnedMeshes().end(), [&](auto const& path) { executor.submit(createSkinnedMeshJob(path)); });
        std::for_each(man.staticMeshes().begin(), man.staticMeshes().end(), [&](auto const& path) { executor.submit(createStaticMeshJob(path)); });

        executor.wait();

        _failedJobs = executor.failedJobs();

        if (!_failedJobs.empty())
        {
            return makeErrorCode(ErrorCode::AssetProcessingFailed);
        }

        // TODO: persist the cooked mesh once PAK writing is implemented.

        return {};
    }

    auto Processor::failedJobs() const -> std::vector<JobResult> const&
    {
        return _failedJobs;
    }
} // namespace ac
