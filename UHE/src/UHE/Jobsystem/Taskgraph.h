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
    
    // IDs of tasks that depend on this task completing
    std::vector<TaskID> dependents;

    TaskNode() = default;

    // We must provide a move constructor because std::atomic is not move-constructible by default.
    // std::vector requires its elements to be move-constructible for when it resizes.
    TaskNode(TaskNode&& other) noexcept
        : entryPoint(other.entryPoint),
          pContext(other.pContext),
          dependenciesRemaining(other.dependenciesRemaining.load(std::memory_order_relaxed)),
          dependents(std::move(other.dependents))
    {}

    TaskNode& operator=(TaskNode&& other) noexcept
    {
        entryPoint = other.entryPoint;
        pContext = other.pContext;
        dependenciesRemaining.store(other.dependenciesRemaining.load(std::memory_order_relaxed), std::memory_order_relaxed);
        dependents = std::move(other.dependents);
        return *this;
    }
};

class TaskGraph 
{
public:
    TaskGraph() = default;
    ~TaskGraph() = default;

    // Allocate a task node in a flat, cache-friendly array
    TaskID CreateTask(JobFn fn, void* pContext = nullptr);

    // Task A must finish before Task B can start
    void AddDependency(TaskID a, TaskID b);

    // Execute the graph using your Job System
    void Execute(UheJobsystem& jobSystem);

    // Clears the graph for the next frame
    void Reset();

private:
    void DispatchTask(UheJobsystem& jobSystem, TaskID id);

    std::vector<TaskNode> m_nodes;
    std::atomic<uint32_t> m_totalTasksRemaining{0};
};

} // namespace UHE::Jobsystem
