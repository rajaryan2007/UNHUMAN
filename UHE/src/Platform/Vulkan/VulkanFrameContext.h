#pragma once
#include <vulkan/vulkan_raii.hpp>
#include "Platform/Vulkan/VulkanCommandBuffer.h"
#include "Platform/Vulkan/VulkanCommandPool.h"
#include "UHE/RHI/DeletionQueue.h"

namespace UHE::RHI::VULKAN
{

class VulkanFrameContext
{
public:
    VulkanFrameContext() = default;

    void Init(const vk::raii::Device& device, u32 queueFamilyIndex);

    void Cleanup();

    inline VulkanCommandBuffer& GetCommandBuffer() { return commandBuffer; }
    inline vk::raii::Semaphore& GetimageAvailableSemaphore() { return imageAvailableSemaphore; }
    inline vk::raii::Semaphore& GetrenderFinishedSemaphore() { return renderFinishedSemaphore; }
    inline vk::raii::Fence& GetInFlightFence() { return inFlightFence; }
    inline DeletionQueue& GetDeletionQueue() { return deletionQueue; }

private:
    VulkanCommandPool commandPool;
    VulkanCommandBuffer commandBuffer;
    vk::raii::Semaphore imageAvailableSemaphore = nullptr;
    vk::raii::Semaphore renderFinishedSemaphore = nullptr;
    vk::raii::Fence inFlightFence = nullptr;

    // Deferred resource destruction queue per-frame
    DeletionQueue deletionQueue;
};

} // namespace UHE::RHI::VULKAN
