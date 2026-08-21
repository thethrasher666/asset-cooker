//
// Copyright (c) 2026 Jamie Kenyon. All Rights Reserved.
//

#include "ac-io/job.hxx"
#include "ac-io/error.hxx"

#include <proc-lib/process.hxx>

#include <format>
#include <stdexcept>

namespace ac
{
    auto runProcess(std::filesystem::path const& executable, std::vector<std::string> const& arguments) -> JobOutcome
    {
        pl::Process process;

        if (auto const startError = process.start(executable, arguments); startError)
        {
            return JobOutcome{ .error = makeErrorCode(ErrorCode::ProcessFailedToStart), .detail = startError.message() };
        }

        auto const result = process.wait();

        if (result.succeeded())
        {
            return {};
        }

        auto const code = [&]
        {
            switch (result.status)
            {
            case pl::ProcessResult::Status::FailedToStart:
                return ErrorCode::ProcessFailedToStart;

            case pl::ProcessResult::Status::NonZeroExit:
                return ErrorCode::ProcessNonZeroExit;

            case pl::ProcessResult::Status::Terminated:
                return ErrorCode::ProcessTerminated;

            case pl::ProcessResult::Status::Killed:
                return ErrorCode::ProcessKilled;

            case pl::ProcessResult::Status::Crashed:
                return ErrorCode::ProcessCrashed;

            case pl::ProcessResult::Status::Success:
            case pl::ProcessResult::Status::Error:
            default:
                return ErrorCode::ProcessFailed;
            }
        }();

        auto detail = result.exitCode >= 0 ? std::format("exit code {}", result.exitCode) : std::string{};

        if (!result.message.empty())
        {
            detail = detail.empty() ? result.message : std::format("{}: {}", detail, result.message);
        }

        return JobOutcome{ .error = makeErrorCode(code), .detail = std::move(detail) };
    }

    auto JobResult::succeeded() const -> bool
    {
        return !outcome.error;
    }

    auto JobExecutor::executeJob(Job job) -> JobResult
    {
        auto outcome = job.work(job.input, job.output);

        return JobResult{ .job = std::move(job), .outcome = std::move(outcome) };
    }

    auto describe(JobResult const& result) -> std::string
    {
        if (result.succeeded())
        {
            return std::format("{}: succeeded", result.job.name);
        }

        return result.outcome.detail.empty() ? std::format("{}: {}", result.job.name, result.outcome.error.message()) :
                                               std::format("{}: {}: {}", result.job.name, result.outcome.error.message(), result.outcome.detail);
    }

    JobExecutor::JobExecutor(std::size_t workerCount)
    {
        if (workerCount == 0)
        {
            workerCount = 1;
        }

        _workers.reserve(workerCount);

        for (std::size_t index = 0; index < workerCount; ++index)
        {
            _workers.emplace_back([this]() -> void { workerLoop(); });
        }
    }

    JobExecutor::~JobExecutor()
    {
        wait();

        {
            std::lock_guard lock(_mutex);
            _stopping = true;
        }

        _jobAvailable.notify_all();

        for (auto& worker : _workers)
        {
            worker.join();
        }
    }

    void JobExecutor::submit(Job job)
    {
        {
            std::lock_guard lock(_mutex);

            if (_stopping)
            {
                return;
            }

            _outputs.emplace_back(job.output);
            _jobs.push(std::move(job));
        }

        _jobAvailable.notify_one();
    }

    void JobExecutor::wait()
    {
        std::unique_lock lock(_mutex);

        _allJobsFinished.wait(lock, [this]() -> bool { return _jobs.empty() && _activeJobs == 0; });
    }

    auto JobExecutor::failedJobs() const -> std::vector<JobResult>
    {
        std::lock_guard lock(_mutex);

        return _failedJobs;
    }

    void JobExecutor::workerLoop()
    {
        while (true)
        {
            Job job;

            {
                std::unique_lock lock(_mutex);

                _jobAvailable.wait(lock, [this]() -> bool { return _stopping || !_jobs.empty(); });

                if (_jobs.empty())
                {
                    return;
                }

                job = std::move(_jobs.front());
                _jobs.pop();

                ++_activeJobs;
            }

            auto result = executeJob(std::move(job));

            {
                std::lock_guard lock(_mutex);

                --_activeJobs;

                if (!result.succeeded())
                {
                    _failedJobs.push_back(std::move(result));
                }

                if (_jobs.empty() && _activeJobs == 0)
                {
                    _allJobsFinished.notify_all();
                }
            }
        }
    }
} // namespace ac
