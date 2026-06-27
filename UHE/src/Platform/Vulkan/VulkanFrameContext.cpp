
#include "uhepch.h"
#include "VulkanFrameContext.h"
#include <queue>
#include "vulkan/vulkan.hpp"

namespace UHE::RHI::VULKAN
{

void VulkanFrameContext::Init(vk::raii::Device& device, u32 queueFamilyIndex)
{
    commandPool.Init(device, queueFamilyIndex, vk::CommandPoolCreateFlagBits::eResetCommandBuffer);
    commandBuffer.Allocate(device, commandPool, true);

    vk::SemaphoreCreateInfo semaphoreInfo{
        .flags = {}
    };
    vk::FenceCreateInfo fenceInfo{
        .flags = vk::FenceCreateFlagBits::eSignaled
    };

    imageAvailableSemaphore = vk::raii::Semaphore(device, semaphoreInfo);
    inFlightFence = vk::raii::Fence(device, fenceInfo);
}

void VulkanFrameContext::Cleanup()
{
    deletionQueue.Flush();
    inFlightFence.clear();
    imageAvailableSemaphore.clear();
    commandBuffer.Free();
    commandPool.CleanUp();
}

} // namespace UHE::RHI::VULKAN
