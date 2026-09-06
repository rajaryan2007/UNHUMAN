#include "Taskgraph.h"

namespace UHE::Jobsystem
{

TaskID TaskGraph::CreateTask(JobFn fn, void* pContext)
{
    TaskID id = static_cast<TaskID>(m_nodes.size());
    auto* pNode = reinterpret_cast<TaskNode*>(m_nodes.data() + id);
    pNode->entryPoint = fn;
    pNode->pContext = pContext;
    return id;
}

void TaskGraph::AddDependency(TaskID a, TaskID b)
{
    // Task A must finish before Task B can start.
    // We add B to A's dependents list.
    m_nodes[a].dependents.push_back(b);

    // We increase B's dependency counter by 1.
    m_nodes[b].dependenciesRemaining.fetch_add(1, std::memory_order_relaxed);
}

void TaskGraph::Execute(UheJobsystem& jobSystem)
{
    m_totalTasksRemaining.store(static_cast<uint32_t>(m_nodes.size()), std::memory_order_relaxed);

    // Find all root nodes (tasks with 0 dependencies) and kick them immediately
    for (TaskID i = 0; i < m_nodes.size(); ++i)
    {
        if (m_nodes[i].dependenciesRemaining.load(std::memory_order_relaxed) == 0)
        {
            DispatchTask(jobSystem, i);
        }
    }

    // Wait until all tasks in the graph have completed
    while (m_totalTasksRemaining.load(std::memory_order_acquire) > 0)
    {
        // Simple busy-wait with yield.
        // Optimization: In a production engine, the main thread can help process
        // tasks from the jobSystem while waiting here!
        std::this_thread::yield();
    }
}

void TaskGraph::Reset()
{
    m_nodes.clear();
}

void TaskGraph::DispatchTask(UheJobsystem& jobSystem, TaskID id)
{
    // We push a small lambda to the job system
    jobSystem.Execute(
        [this, &jobSystem, id]()
        {
            TaskNode& node = m_nodes[id];

            // 1. Run the actual engine work
            if (node.entryPoint)
            {
                node.entryPoint(node.pContext);
            }

            // 2. Notify all dependent tasks
            for (TaskID dependentId : node.dependents)
            {
                TaskNode& depNode = m_nodes[dependentId];

                // fetch_sub returns the value BEFORE the subtraction.
                // If it returns 1, that means the counter is about to hit 0!
                if (depNode.dependenciesRemaining.fetch_sub(1, std::memory_order_acq_rel) == 1)
                {
                    DispatchTask(jobSystem, dependentId);
                }
            }

            // 3. Mark total progress as complete for this task
            m_totalTasksRemaining.fetch_sub(1, std::memory_order_release);
        });
}

} // namespace UHE::Jobsystem
