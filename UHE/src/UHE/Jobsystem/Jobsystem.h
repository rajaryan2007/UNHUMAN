#pragma once
#include <cstdint>
#include <functional>
#include <atomic>
#include <vector>
#include <deque>
#include <thread>
#include <mutex>
#include <condition_variable>

namespace UHE::Jobsystem
{

// Self-contained alias so this header compiles even when included without the
// engine-wide precompiled header (where u32 comes from UHE/Core/Core.h).
using u32 = std::uint32_t;

// A lightweight counter to wait for a group of jobs to finish.
// This is the core of our "Task Graph" - jobs can depend on counters!
struct JobCounter
{
    std::atomic<int> count{0};
};

// Represents a single task to execute
using JobFunction = std::function<void()>;

class UheJobsystem
{
public:
    UheJobsystem() = default;
    ~UheJobsystem() = default;

    // Initializes the thread pool.
    // Will detect hardware cores (very important for Android big/LITTLE architectures)
    void Init();

    // Safely joins all threads
    void ShutDown();

    // Kick a single job to the thread pool.
    // If a counter is provided, it increments the counter before scheduling,
    // and decrements it when the job finishes.
    void Execute(JobFunction job, JobCounter* counter = nullptr);

    // Schedules 'jobCount' jobs by calling 'fn(i)' for i in [0, jobCount).
    // The range is split into chunks so each worker gets a contiguous slice
    // (less contention than one job per element for cheap iterations).
    // Blocks until ALL iterations have completed.
    void ParallelFor(u32 jobCount, const std::function<void(u32)>& fn, u32 minBatchSize = 1);

    // Blocks the current thread until the counter reaches zero.
    // While waiting, the calling thread STEALS jobs from the queue and executes
    // them itself, so it never wastes CPU time.
    void Wait(const JobCounter* counter);

    // Blocks until every job kicked so far has completed.
    // Unlike ShutDown, the jobsystem keeps running afterwards.
    void WaitForAll();

    // True while any job kicked through this jobsystem is still running/queued.
    bool IsBusy() const;

    // Get number of worker threads (Useful for allocating 1 Vulkan CommandPool per thread!)
    u32 GetThreadCount() const;

    // Get the ID of the current thread (0 to N). Useful for indexing into thread-local arrays.
    static u32 GetCurrentThreadIndex();

    // Tries to pop ONE queued job and executes it on the calling thread.
    // Returns false if the queue was empty. Used by worker threads and by
    // the main thread while helping inside Wait()/WaitForAll()/TaskGraph.
    bool TryPopJob(std::pair<JobFunction, JobCounter*>& outJob);

private:
    void WorkerThread(u32 threadIndex);

    std::vector<std::thread> m_Workers;

    // A simple thread-safe queue for jobs.
    // In the future, we will upgrade this to a lock-free work-stealing deque for max performance.
    std::deque<std::pair<JobFunction, JobCounter*>> m_JobQueue;

    std::mutex m_QueueMutex;
    std::condition_variable m_WakeCondition;

    std::atomic<bool> m_IsRunning{false};

    // Number of jobs that have been scheduled but not yet finished.
    std::atomic<int> m_JobsInFlight{0};
};

} // namespace UHE::Jobsystem
