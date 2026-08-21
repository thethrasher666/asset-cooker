//
// Copyright (c) 2026 Jamie Kenyon. All Rights Reserved.
//

#include "ac-io/job.hxx"

namespace ac
{
    JobExecutor::JobExecutor(std::size_t workerCount)
    {
        if (workerCount == 0)
        {
            throw std::invalid_argument("JobExecutor requires at least one worker");
        }

        _workers.reserve(workerCount);

        for (std::size_t index = 0; index < workerCount; ++index)
        {
            _workers.emplace_back([this](std::stop_token stopToken) -> void { workerLoop(stopToken); });
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

        // std::jthread automatically requests stop and joins.
    }

    void JobExecutor::submit(Job job)
    {
        {
            std::lock_guard lock(_mutex);

            if (_stopping)
            {
                throw std::runtime_error("Cannot submit a job to a stopping executor");
            }

            _jobs.push(std::move(job));
        }

        _jobAvailable.notify_one();
    }

    void JobExecutor::wait()
    {
        std::unique_lock lock(_mutex);

        _allJobsFinished.wait(lock, [this]() -> bool { return _jobs.empty() && _activeJobs == 0; });
    }

    auto JobExecutor::results() const -> std::vector<JobResult>
    {
        std::lock_guard lock(_mutex);

        return _results;
    }

    auto JobExecutor::failedJobs() const -> std::vector<JobResult>
    {
        std::lock_guard lock(_mutex);

        std::vector<JobResult> failures;

        for (const auto& result : _results)
        {
            if (!result.succeeded())
            {
                failures.push_back(result);
            }
        }

        return failures;
    }

    auto JobExecutor::pendingJobCount() const -> std::size_t
    {
        std::lock_guard lock(_mutex);

        return _jobs.size();
    }

    auto JobExecutor::activeJobCount() const -> std::size_t
    {
        std::lock_guard lock(_mutex);

        return _activeJobs;
    }

    void JobExecutor::workerLoop(std::stop_token stopToken)
    {
        while (!stopToken.stop_requested())
        {
            Job job;

            {
                std::unique_lock lock(_mutex);

                _jobAvailable.wait(lock, [this, &stopToken]() -> bool { return stopToken.stop_requested() || _stopping || !_jobs.empty(); });

                if (stopToken.stop_requested())
                {
                    return;
                }

                if (_jobs.empty())
                {
                    if (_stopping)
                    {
                        return;
                    }

                    continue;
                }

                job = std::move(_jobs.front());
                _jobs.pop();

                ++_activeJobs;
            }

            auto result = executeJob(std::move(job));

            {
                std::lock_guard lock(_mutex);

                _results.push_back(std::move(result));

                --_activeJobs;

                if (_jobs.empty() && _activeJobs == 0)
                {
                    _allJobsFinished.notify_all();
                }
            }
        }
    }

    auto JobExecutor::executeJob(Job job) -> JobResult
    {
        JobResult result;

        result.outcome = std::visit([](auto& work) -> std::variant<pl::ProcessResult, std::error_code>
        {
            using T = std::decay_t<decltype(work)>;
            if constexpr (std::is_same_v<T, ProcessWork>)
            {
                return runProcess(work.executable, work.arguments);
            }
            else
            {
                return work();
            }
        }, job.work);

        result.job = std::move(job);

        return result;
    }

    auto JobExecutor::runProcess(std::filesystem::path const& executable, std::vector<std::string> arguments) -> pl::ProcessResult
    {
        auto process = std::make_unique<pl::Process>();

        if (auto result = process->start(executable, arguments); result)
        {
            pl::ProcessResult processResult{ .status = pl::ProcessResult::Status::FailedToStart };
            return processResult;
        }

        return process->wait();
    }
} // namespace ac
