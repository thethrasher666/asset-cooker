//
// Copyright (c) 2026 Jamie Kenyon. All Rights Reserved.
//

#pragma once

#include <condition_variable>
#include <filesystem>
#include <functional>
#include <mutex>
#include <queue>
#include <string>
#include <system_error>
#include <thread>
#include <vector>

namespace ac
{
    /// The outcome of executing a single Job's work.
    struct JobOutcome
    {
        std::error_code error;  ///< Falsy (default-constructed) on success.
        std::string     detail; ///< Additional diagnostic detail (e.g. a process's exit code/message); empty if none.
    };

    /// A unit of work: given a job's input and output paths, performs the work and returns the outcome.
    using Work = std::function<JobOutcome(std::filesystem::path const& input, std::filesystem::path const& output)>;

    /// Runs an external process to completion and converts the result into a JobOutcome.
    /// Intended for use inside a Work callable that needs to shell out to a tool.
    /// \param executable The executable to run.
    /// \param arguments Arguments passed to the executable.
    /// \return The outcome of running the process.
    [[nodiscard]] auto runProcess(std::filesystem::path const& executable, std::vector<std::string> const& arguments) -> JobOutcome;

    /// Describes a unit of work to be executed by a JobExecutor.
    struct Job
    {
        std::string            name;   ///< A human-readable name for the job.
        std::filesystem::path  input;  ///< Fully-qualified path to the job's input file.
        std::filesystem::path  output; ///< Fully-qualified path to the job's expected output file.
        Work                   work;   ///< The work this job performs.
    };

    /// Contains the result of executing a job.
    struct JobResult
    {
        Job        job;     ///< The job that was executed.
        JobOutcome outcome; ///< The outcome of running the job's work.

        /// Determines whether the job completed successfully.
        /// \return true if the job succeeded; otherwise false.
        [[nodiscard]] auto succeeded() const -> bool;
    };

    /// Produces a human-readable description of a job's outcome, including process status, exit code,
    /// and any failure message, suitable for reporting to a user.
    /// \param result The job result to describe.
    /// \return A description of the job result.
    [[nodiscard]] auto describe(JobResult const& result) -> std::string;

    /// Executes process jobs using a fixed number of worker threads.
    /// Jobs may be submitted from multiple threads. The executor limits the
    /// number of concurrently running jobs to the number of worker threads
    /// specified during construction.
    class JobExecutor
    {
    public:
        /// Creates a job executor with the specified number of workers.
        /// \param workerCount The maximum number of jobs that may execute concurrently.
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

        /// Returns the results of jobs that have completed but did not succeed.
        /// \return A copy of the failed job results recorded so far.
        [[nodiscard]] auto failedJobs() const -> std::vector<JobResult>;

    private:
        /// Runs on each worker thread, pulling jobs from the queue and executing them until
        /// the executor is stopping and the queue has been drained.
        void workerLoop();

        /// Runs the work described by a job and packages the outcome.
        /// \param job The job to execute.
        /// \return The result of executing the job.
        [[nodiscard]] static auto executeJob(Job job) -> JobResult;

        mutable std::mutex                 _mutex;            ///< Protects all the members below.
        std::condition_variable            _jobAvailable;     ///< Signalled when a job is queued or the executor starts stopping.
        std::condition_variable            _allJobsFinished;  ///< Signalled when the queue is empty and no job is executing.
        std::queue<Job>                    _jobs;             ///< Jobs waiting to be picked up by a worker.
        std::vector<std::thread>           _workers;          ///< The worker threads.
        std::vector<JobResult>             _failedJobs;       ///< Results of jobs that completed unsuccessfully.
        std::size_t                        _activeJobs = 0;   ///< The number of jobs currently being executed.
        bool                               _stopping = false; ///< Set once the executor has begun shutting down.
        std::vector<std::filesystem::path> _outputs;          ///< Fully-qualified paths of files produced by the jobs.
    };
} // namespace ac
