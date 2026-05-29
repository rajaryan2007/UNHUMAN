
#include "uhepch.h"
#include "VulkanFrameContext.h"

namespace UHE::RHI::VULKAN {

void VulkanFrameContext::Init(const vk::raii::Device& device, u32 queueFamilyIndex) {
    vk::CommandPoolCreateInfo poolInfo{};
    poolInfo.queueFamilyIndex = queueFamilyIndex;
    poolInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;

    commandPool = vk::raii::CommandPool(device, poolInfo);

    vk::CommandBufferAllocateInfo allocInfo{};
    allocInfo.commandPool = *commandPool;
    allocInfo.level = vk::CommandBufferLevel::ePrimary;
    allocInfo.commandBufferCount = 1;

    vk::raii::CommandBuffers buffers(device, allocInfo);
    commandBuffer = std::move(buffers[0]);

    vk::SemaphoreCreateInfo semaphoreInfo{};
    vk::FenceCreateInfo fenceInfo{};
    fenceInfo.flags = vk::FenceCreateFlagBits::eSignaled; // Signaled by default so first wait doesn't block

    imageAvailableSemaphore = vk::raii::Semaphore(device, semaphoreInfo);
    renderFinishedSemaphore = vk::raii::Semaphore(device, semaphoreInfo);
    inFlightFence = vk::raii::Fence(device, fenceInfo);
}

void VulkanFrameContext::Cleanup() {
    deletionQueue.Flush();
    inFlightFence.clear();
    imageAvailableSemaphore.clear();
    renderFinishedSemaphore.clear();
    commandBuffer.clear();
    commandPool.clear();
}

} // namespace UHE::RHI
