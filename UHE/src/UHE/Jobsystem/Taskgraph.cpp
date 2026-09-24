#include "Taskgraph.h"
#include <algorithm>

namespace UHE::Jobsystem
{

TaskID TaskGraph::CreateTask(JobFn fn, void* pContext)
{
    // Reserve() copies/moves existing nodes but never reallocates while tasks
    // are being created, so the push_back below can never move the array out
    // from under an id captured elsewhere during graph building.
    m_nodes.reserve(m_nodes.size() + 1);
    const TaskID id = static_cast<TaskID>(m_nodes.size());
    TaskNode& node = m_nodes.emplace_back();
    node.entryPoint = fn;
    node.pContext = pContext;
    return id;
}

void TaskGraph::AddDependency(TaskID a, TaskID b)
{
    // Task A must finish before Task B can start.
    // We add B to A's dependents list.
    m_nodes[a].dependents.push_back(b);

    // We increase B's dependency counters by 1: the atomic used at runtime and
    // the immutable total used to re-arm the graph for the next Execute().
    TaskNode& depNode = m_nodes[b];
    depNode.dependencyCount += 1;
    depNode.dependenciesRemaining.fetch_add(1, std::memory_order_relaxed);
}

void TaskGraph::Execute(UheJobsystem& jobSystem)
{
    const uint32_t totalTasks = static_cast<uint32_t>(m_nodes.size());
    m_totalTasksRemaining.store(totalTasks, std::memory_order_relaxed);

    // Re-arm per-task dependency counters from the immutable incoming counts,
    // so the graph can be executed repeatedly (e.g. once per frame).
    for (TaskID i = 0; i < totalTasks; ++i)
        m_nodes[i].dependenciesRemaining.store(m_nodes[i].dependencyCount, std::memory_order_relaxed);

    // Find all root nodes and kick them immediately.
    // IMPORTANT: check the immutable dependencyCount, NOT the mutable
    // dependenciesRemaining. Workers resolve dependents while this scan is
    // still running, so a non-root's runtime counter can hit 0 mid-scan; if
    // we checked that, the task would be dispatched twice (once here, once by
    // the dependency resolver) and run twice. dependencyCount == 0 tasks can
    // never appear in a dependents list, so dispatch ownership is exclusive.
    for (TaskID i = 0; i < totalTasks; ++i)
    {
        if (m_nodes[i].dependencyCount == 0)
        {
            jobSystem.Execute(
                [this, &jobSystem, i]()
                {
                    RunTask(jobSystem, i);
                });
        }
    }

    // Wait until all tasks in the graph have completed.
    // While waiting, the main thread HELPS by executing already-dispatched
    // tasks from the job system queue instead of spinning idle.
    // (A dispatched task is genuinely ready; tasks still waiting on
    // dependencies are never in the queue, so helping here stays correct.)
    // Signed counter: a dispatch-ownership bug would drive it negative and
    // exit this loop (tests fail loudly) instead of wrapping to ~4 billion
    // and hanging forever.
    while (m_totalTasksRemaining.load(std::memory_order_acquire) > 0)
    {
        std::pair<JobFunction, JobCounter*> job;
        if (!jobSystem.TryPopJob(job))
        {
            // Queue is empty: remaining tasks are either running on workers
            // or will be dispatched when their dependencies resolve.
            std::this_thread::yield();
        }
    }
}

void TaskGraph::Reset()
{
    m_nodes.clear();
}

void TaskGraph::OnDependencyResolved(UheJobsystem& jobSystem, TaskID id)
{
    // Notify all dependent tasks
    for (TaskID dependentId : m_nodes[id].dependents)
    {
        TaskNode& depNode = m_nodes[dependentId];

        // fetch_sub returns the value BEFORE the subtraction.
        // If it returns 1, that means the counter is about to hit 0!
        const uint32_t prev = depNode.dependenciesRemaining.fetch_sub(1, std::memory_order_acq_rel);
        if (prev == 1)
        {
            jobSystem.Execute(
                [this, &jobSystem, dependentId]()
                {
                    RunTask(jobSystem, dependentId);
                });
        }
    }

    // Mark total progress as complete for this task.
    // NOTE: this decrement must be the LAST access to graph state here. The
    // acquire load in Execute() observing 0 gives main the right to destroy
    // the graph immediately - any access after this fetch_sub (e.g. a debug
    // assert reading the counter) would race with that destruction. This was
    // caught by ThreadSanitizer.
    m_totalTasksRemaining.fetch_sub(1, std::memory_order_release);
}

void TaskGraph::RunTask(UheJobsystem& jobSystem, TaskID id)
{
    TaskNode& node = m_nodes[id];

    // 1. Run the actual engine work
    if (node.entryPoint)
    {
        node.entryPoint(node.pContext);
    }

    // 2. Resolve dependents + progress
    OnDependencyResolved(jobSystem, id);
}

} // namespace UHE::Jobsystem
