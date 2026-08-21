//
// Copyright (c) 2026 Jamie Kenyon. All Rights Reserved.
//

#pragma once

#include <proc-lib/process.hxx>

#include <condition_variable>
#include <cstddef>
#include <filesystem>
#include <functional>
#include <mutex>
#include <optional>
#include <queue>
#include <stdexcept>
#include <stop_token>
#include <string>
#include <thread>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

namespace ac
{
    /// Describes a job that runs an external executable.
    struct ProcessWork
    {
        std::filesystem::path    executable; ///< The executable to run.
        std::vector<std::string> arguments;  ///< Arguments passed to the executable.
    };

    /// Describes a job that calls an in-process function.
    using FunctionWork = std::function<std::error_code()>;

    /// Describes a unit of work to be executed by a JobExecutor.
    struct Job
    {
        std::string                             name; ///< A human-readable name for the job.
        std::variant<ProcessWork, FunctionWork> work; ///< The work this job performs.
    };

    /// Contains the result of executing a job.
    struct JobResult
    {
        Job                                              job;     ///< The job that was executed.
        std::variant<pl::ProcessResult, std::error_code> outcome; ///< The outcome of a ProcessWork job, or the error_code returned by a FunctionWork job.

        /// Determines whether the job completed successfully.
        /// \return true if the job succeeded; otherwise false.
        [[nodiscard]] auto succeeded() const -> bool
        {
            return std::visit(
            [](auto const& value) -> bool
            {
                using T = std::decay_t<decltype(value)>;
                if constexpr (std::is_same_v<T, pl::ProcessResult>)
                {
                    return value.succeeded();
                }
                else
                {
                    return !value;
                }
            },
            outcome);
        }
    };

    /// Executes process jobs using a fixed number of worker threads.
    /// Jobs may be submitted from multiple threads. The executor limits the
    /// number of concurrently running jobs to the number of worker threads
    /// specified during construction.
    class JobExecutor
    {
    public:
        /// Creates a job executor with the specified number of workers.
        /// \param workerCount The maximum number of jobs that may execute concurrently.
        /// \throws std::invalid_argument If @p workerCount is zero.
        explicit JobExecutor(std::size_t workerCount = std::thread::hardware_concurrency());

        /// Waits for all queued jobs to complete and stops the workers.
        ~JobExecutor();

        JobExecutor(JobExecutor const&) = delete;
        auto operator=(JobExecutor const&) -> JobExecutor& = delete;

        JobExecutor(JobExecutor&&) = delete;
        auto operator=(JobExecutor&&) -> JobExecutor& = delete;

        /// Adds a job to the execution queue.
        /// \param job The job to execute.
        /// \throws std::runtime_error If the executor is stopping.
        void submit(Job job);

        /// Waits until all currently submitted jobs have completed.
        /// This function returns when both the pending job queue is empty and
        /// no worker is currently executing a job.
        void wait();

        /// Returns the results of all completed jobs.
        /// The returned vector is a copy of the internally stored results.
        /// \return The results of all completed jobs.
        [[nodiscard]] auto results() const -> std::vector<JobResult>;

        /// Returns the results of all failed jobs.
        /// \return A vector containing only jobs that did not succeed.
        [[nodiscard]] auto failedJobs() const -> std::vector<JobResult>;

        /// Returns the number of jobs currently waiting to execute.
        /// \return The number of pending jobs.
        [[nodiscard]] auto pendingJobCount() const -> std::size_t;

        /// Returns the number of jobs currently executing.
        /// \return The number of active jobs.
        [[nodiscard]] auto activeJobCount() const -> std::size_t;

    private:
        void        workerLoop(std::stop_token stopToken);
        static auto executeJob(Job job) -> JobResult;
        static auto runProcess(std::filesystem::path const& executable, std::vector<std::string> arguments) -> pl::ProcessResult;

    private:
        mutable std::mutex        _mutex;            /// Mutex protecting the executor state.
        std::condition_variable   _jobAvailable;     /// Signalled when a new job becomes available.
        std::condition_variable   _allJobsFinished;  /// Signalled when all submitted jobs have completed.
        std::queue<Job>           _jobs;             /// Queue of jobs waiting to execute.
        std::vector<JobResult>    _results;          /// Results of completed jobs.
        std::vector<std::jthread> _workers;          /// Worker threads executing jobs.
        std::size_t               _activeJobs = 0;   /// Number of jobs currently executing.
        bool                      _stopping = false; /// Indicates that the executor is shutting down.
    };
} // namespace ac
