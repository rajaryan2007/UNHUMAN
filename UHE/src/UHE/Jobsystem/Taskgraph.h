#pragma once
#include "Jobsystem.h"
#include <vector>
#include <atomic>
#include <cstdint>

namespace UHE::Jobsystem
{

// A function pointer type for tasks
using JobFn = void (*)(void* pContext);
using TaskID = uint32_t;

struct TaskNode
{
    JobFn entryPoint = nullptr;
    void* pContext = nullptr;

    // Number of prerequisites remaining before this task can run
    std::atomic<uint32_t> dependenciesRemaining{0};

    // Total number of incoming dependencies (immutable after graph build).
    // Used to re-arm 'dependenciesRemaining' when the graph is re-executed.
    uint32_t dependencyCount = 0;

    // IDs of tasks that depend on this task completing
    std::vector<TaskID> dependents;

    TaskNode() = default;

    // We must provide a move constructor because std::atomic is not move-constructible by default.
    // std::vector requires its elements to be move-constructible for when it resizes.
    TaskNode(TaskNode&& other) noexcept
        : entryPoint(other.entryPoint),
          pContext(other.pContext),
          dependenciesRemaining(other.dependenciesRemaining.load(std::memory_order_relaxed)),
          dependencyCount(other.dependencyCount),
          dependents(std::move(other.dependents))
    {}

    TaskNode& operator=(TaskNode&& other) noexcept
    {
        entryPoint = other.entryPoint;
        pContext = other.pContext;
        dependenciesRemaining.store(other.dependenciesRemaining.load(std::memory_order_relaxed), std::memory_order_relaxed);
        dependencyCount = other.dependencyCount;
        dependents = std::move(other.dependents);
        return *this;
    }
};

class TaskGraph
{
public:
    TaskGraph() = default;
    ~TaskGraph() = default;

    // Allocate a task node in a flat, cache-friendly array.
    // Reserves capacity up-front, so AddDependency() must only be called
    // between graph building and Execute().
    TaskID CreateTask(JobFn fn, void* pContext = nullptr);

    // Task A must finish before Task B can start
    void AddDependency(TaskID a, TaskID b);

    // Execute the graph using your Job System.
    // The main thread helps execute ready tasks while waiting, instead of
    // spinning idle.
    void Execute(UheJobsystem& jobSystem);

    // Clears the graph for the next frame
    void Reset();

private:
    // Dispatches dependents of 'id' that just finished and updates progress.
    void OnDependencyResolved(UheJobsystem& jobSystem, TaskID id);

    // Runs one task and resolves its dependents. Used by the job lambda.
    void RunTask(UheJobsystem& jobSystem, TaskID id);

    std::vector<TaskNode> m_nodes;

    // Signed so a dispatch-ownership bug shows up as a negative value
    // (visible in tests/debugger) instead of wrapping around to a huge number.
    std::atomic<int32_t> m_totalTasksRemaining{0};
};

} // namespace UHE::Jobsystem
