#pragma once
#include <span>
#include <vulkan/vulkan_raii.hpp>
#include "Platform/Vulkan/VulkanTypes.h"

namespace UHE::RHI::VULKAN
{
class VulkanContext;

using CommandBufferRef = vk::CommandBuffer;

// Neutral description of one queue submission: what to wait on, what to record,
// what to signal. The encoder turns this into vkQueueSubmit2 or vkQueueSubmit.
struct SubmitInfo
{
    std::span<const SemaphoreWait> Waits;
    std::span<const CommandBufferRef> CommandBuffers;
    std::span<const SemaphoreSignal> Signals;
};

class VulkanSubmitEncoder
{
public:
    VulkanSubmitEncoder() = default;
    VulkanSubmitEncoder(const VulkanSubmitEncoder&) = delete;
    VulkanSubmitEncoder operator=(const VulkanSubmitEncoder&) = delete;
    ~VulkanSubmitEncoder() = default;

    void Init(VulkanContext* context);
    void Shutdown();

    void Submit(vk::raii::Queue& queue, const SubmitInfo& info, vk::Fence fence = nullptr);

private:
    VulkanContext* m_Context = nullptr;
};

} // namespace UHE::RHI::VULKAN
