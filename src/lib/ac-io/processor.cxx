//
// Copyright (c) 2026 Jamie Kenyon. All Rights Reserved.
//

#include "ac-io/error.hxx"
#include "ac-io/job.hxx"
#include "ac-io/mesh-processor.hxx"
#include "ac-io/processor.hxx"
#include "ac-io/texture-processor.hxx"

namespace ac
{
    auto Processor::run(Manifest const & manifest) -> std::error_code
    {
        // meshProcessor and textureProcessor must outlive executor: the jobs it runs
        // hold references to them, and both submit loops below finish before executor
        // is destroyed (its destructor blocks until all jobs complete).
        JobExecutor executor;

        MeshProcessor    meshProcessor;
        TextureProcessor textureProcessor;

        for (auto const & path : manifest.staticMeshes())
        {
            executor.submit(meshProcessor.toJob(path));
        }

        for (auto const & path : manifest.skinnedMeshes())
        {
            executor.submit(meshProcessor.toJob(path));
        }

        for (auto const & path : manifest.albedoMaps())
        {
            executor.submit(textureProcessor.toJob(path, TextureKind::Albedo));
        }

        for (auto const & path : manifest.ambientOcclusionMaps())
        {
            executor.submit(textureProcessor.toJob(path, TextureKind::AmbientOcclusion));
        }

        for (auto const & path : manifest.displacementMaps())
        {
            executor.submit(textureProcessor.toJob(path, TextureKind::Displacement));
        }

        for (auto const & path : manifest.emissiveMaps())
        {
            executor.submit(textureProcessor.toJob(path, TextureKind::Emissive));
        }

        for (auto const & path : manifest.metallicMaps())
        {
            executor.submit(textureProcessor.toJob(path, TextureKind::Metallic));
        }

        for (auto const & path : manifest.normalMaps())
        {
            executor.submit(textureProcessor.toJob(path, TextureKind::Normal));
        }

        for (auto const & path : manifest.opacityMaps())
        {
            executor.submit(textureProcessor.toJob(path, TextureKind::Opacity));
        }

        for (auto const & path : manifest.roughnessMaps())
        {
            executor.submit(textureProcessor.toJob(path, TextureKind::Roughness));
        }

        executor.wait();

        if (auto const failures = executor.failedJobs(); !failures.empty())
        {
            return makeErrorCode(ErrorCode::AssetProcessingFailed);
        }

        // TODO: build the PAK file from executor.results().
        return {};
    }
} // namespace ac
