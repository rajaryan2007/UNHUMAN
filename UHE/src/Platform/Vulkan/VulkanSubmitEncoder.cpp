#include "uhepch.h"
#include "Platform/Vulkan/VulkanSubmitEncoder.h"
#include "Platform/Vulkan/VulkanContext.h"
#include "Platform/Vulkan/VulkanExtensionCheck.h"

namespace UHE::RHI::VULKAN
{

void VulkanSubmitEncoder::Init(VulkanContext* context)
{
    m_Context = context;
}

void VulkanSubmitEncoder::Shutdown()
{
    m_Context = nullptr;
}

void VulkanSubmitEncoder::Submit(vk::raii::Queue& queue, const SubmitInfo& info, vk::Fence fence)
{
    const SyncTier tier = m_Context != nullptr ? m_Context->CheckExtensions->GetSyncTier() : SyncTier::Legacy;

    if (tier == SyncTier::Legacy)
    {
        // v1 submit: binary semaphores and a per-wait stage mask. Timeline values
        // have no representation here and are ignored.
        std::vector<vk::Semaphore> waitSemaphores;
        std::vector<vk::PipelineStageFlags> waitStages;
        waitSemaphores.reserve(info.Waits.size());
        waitStages.reserve(info.Waits.size());
        for (const SemaphoreWait& wait : info.Waits)
        {
            waitSemaphores.push_back(wait.Semaphore);
            waitStages.push_back(ToVkPipelineStage1(wait.WaitStage));
        }

        std::vector<vk::Semaphore> signalSemaphores;
        signalSemaphores.reserve(info.Signals.size());
        for (const SemaphoreSignal& signal : info.Signals)
            signalSemaphores.push_back(signal.Semaphore);

        const std::vector<vk::CommandBuffer> commandBuffers(info.CommandBuffers.begin(), info.CommandBuffers.end());

        const vk::SubmitInfo submitInfo{
            .waitSemaphoreCount = static_cast<u32>(waitSemaphores.size()),
            .pWaitSemaphores = waitSemaphores.data(),
            .pWaitDstStageMask = waitStages.data(),
            .commandBufferCount = static_cast<u32>(commandBuffers.size()),
            .pCommandBuffers = commandBuffers.data(),
            .signalSemaphoreCount = static_cast<u32>(signalSemaphores.size()),
            .pSignalSemaphores = signalSemaphores.data(),
        };

        queue.submit(submitInfo, fence);
        return;
    }

    std::vector<vk::SemaphoreSubmitInfo> waits;
    waits.reserve(info.Waits.size());
    for (const SemaphoreWait& wait : info.Waits)
    {
        waits.push_back({
            .semaphore = wait.Semaphore,
            .value = wait.Value,
            .stageMask = ToVkPipelineStage2(wait.WaitStage),
        });
    }

    std::vector<vk::SemaphoreSubmitInfo> signals;
    signals.reserve(info.Signals.size());
    for (const SemaphoreSignal& signal : info.Signals)
    {
        signals.push_back({
            .semaphore = signal.Semaphore,
            .value = signal.Value,
            .stageMask = vk::PipelineStageFlagBits2::eAllCommands,
        });
    }

    std::vector<vk::CommandBufferSubmitInfo> commandInfos;
    commandInfos.reserve(info.CommandBuffers.size());
    for (const vk::CommandBuffer& commandBuffer : info.CommandBuffers)
        commandInfos.push_back({.commandBuffer = commandBuffer});

    const vk::SubmitInfo2 submitInfo{
        .waitSemaphoreInfoCount = static_cast<uint32_t>(waits.size()),
        .pWaitSemaphoreInfos = waits.data(),
        .commandBufferInfoCount = static_cast<uint32_t>(commandInfos.size()),
        .pCommandBufferInfos = commandInfos.data(),
        .signalSemaphoreInfoCount = static_cast<uint32_t>(signals.size()),
        .pSignalSemaphoreInfos = signals.data(),
    };

    queue.submit2(submitInfo, fence);
}

} // namespace UHE::RHI::VULKAN
