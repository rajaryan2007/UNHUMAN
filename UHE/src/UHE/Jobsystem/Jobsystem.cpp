#include "Jobsystem.h"
#include <algorithm> // For std::max

namespace UHE::Jobsystem
{

// A thread-local variable to store the unique index of the current thread.
// Main thread will be 0 (default initialized). Worker threads will be 1 to N.
thread_local uint32_t t_ThreadIndex = 0;

void UheJobsystem::Init()
{
    // If the system can't determine hardware concurrency, fallback to 4 threads.
    uint32_t numThreads = std::thread::hardware_concurrency();
    if (numThreads == 0)
        numThreads = 4;

    // We leave 1 core for the main thread, so we spawn numThreads - 1 workers
    uint32_t numWorkers = std::max(1u, numThreads - 1);

    m_IsRunning.store(true, std::memory_order_release);

    for (uint32_t i = 0; i < numWorkers; ++i)
    {
        // Thread index 0 is reserved for the main thread, so workers start at 1
        m_Workers.emplace_back(&UheJobsystem::WorkerThread, this, i + 1);
    }
}

void UheJobsystem::ShutDown()
{
    if (!m_IsRunning.load(std::memory_order_acquire))
        return;

    // Signal all threads to stop
    m_IsRunning.store(false, std::memory_order_release);
    m_WakeCondition.notify_all();

    // Wait for all threads to finish their current job and exit
    for (auto& worker : m_Workers)
    {
        if (worker.joinable())
        {
            worker.join();
        }
    }
    m_Workers.clear();
}

void UheJobsystem::Execute(JobFunction job, JobCounter* counter)
{
    if (counter)
    {
        counter->count.fetch_add(1, std::memory_order_relaxed);
    }

    {
        std::lock_guard<std::mutex> lock(m_QueueMutex);
        m_JobQueue.emplace_back(std::move(job), counter);
    }

    // Wake up one sleeping worker thread to take the job
    m_WakeCondition.notify_one();
}

void UheJobsystem::Wait(const JobCounter* counter)
{
    if (!counter)
        return;

    // Busy wait until the counter hits zero.
    // std::this_thread::yield() tells the OS to let other threads run while we wait.
    // LATER OPTIMIZATION: Instead of yielding, the waiting thread should grab jobs
    // from the queue and execute them so it doesn't waste CPU time!
    while (counter->count.load(std::memory_order_relaxed) > 0)
    {
        std::this_thread::yield();
    }
}

uint32_t UheJobsystem::GetThreadCount() const
{
    // Return total threads: Workers + 1 (Main Thread)
    return static_cast<uint32_t>(m_Workers.size() + 1);
}

uint32_t UheJobsystem::GetCurrentThreadIndex()
{
    return t_ThreadIndex;
}

void UheJobsystem::WorkerThread(uint32_t threadIndex)
{
    // Store the unique index in this thread's local storage
    t_ThreadIndex = threadIndex;

    while (m_IsRunning.load(std::memory_order_acquire))
    {
        std::pair<JobFunction, JobCounter*> jobData;

        {
            std::unique_lock<std::mutex> lock(m_QueueMutex);

            // Wait until either the system shuts down, or we have a job in the queue
            m_WakeCondition.wait(lock, [this]()
                                 { return !m_JobQueue.empty() || !m_IsRunning.load(std::memory_order_acquire); });

            // If we woke up because of shutdown and queue is empty, exit thread
            if (!m_IsRunning.load(std::memory_order_acquire) && m_JobQueue.empty())
            {
                break;
            }

            // Grab the next job from the queue
            jobData = std::move(m_JobQueue.front());
            m_JobQueue.erase(m_JobQueue.begin());
        }

        // Execute the job safely without holding the lock!
        if (jobData.first)
        {
            jobData.first();
        }

        // If a counter was attached, decrement it to signal completion
        if (jobData.second)
        {
            jobData.second->count.fetch_sub(1, std::memory_order_release);
        }
    }
}

} // namespace UHE::Jobsystem
