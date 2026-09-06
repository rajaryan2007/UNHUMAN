#pragma once
#include <functional>
#include <atomic>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>

namespace UHE::Jobsystem
{

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

    // Blocks the current thread until the counter reaches zero.
    // OPTIMIZATION LATER: Instead of blocking, the waiting thread can help execute jobs!
    void Wait(const JobCounter* counter);

    // Get number of worker threads (Useful for allocating 1 Vulkan CommandPool per thread!)
    uint32_t GetThreadCount() const;
    
    // Get the ID of the current thread (0 to N). Useful for indexing into thread-local arrays.
    static uint32_t GetCurrentThreadIndex();

private:
    void WorkerThread(uint32_t threadIndex);

    std::vector<std::thread> m_Workers;
    
    // A simple thread-safe queue for jobs.
    // In the future, we will upgrade this to a lock-free work-stealing deque for max performance.
    std::vector<std::pair<JobFunction, JobCounter*>> m_JobQueue;
    
    std::mutex m_QueueMutex;
    std::condition_variable m_WakeCondition;
    
    std::atomic<bool> m_IsRunning{false};
};

} // namespace UHE::Jobsystem
