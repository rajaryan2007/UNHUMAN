#include "uhepch.h"
#include "VulkanCommandPool.h"
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_raii.hpp>
#include "vulkan/vulkan.hpp"

namespace UHE::RHI::VULKAN
{
void VulkanCommandPool::Init(vk::raii::Device& device, u32 queueFamilyIndex, vk::CommandPoolCreateFlags flags)
{
    vk::CommandPoolCreateInfo poolInfo{};
    poolInfo.queueFamilyIndex = queueFamilyIndex;
    poolInfo.flags = flags;

    m_CommandPool = vk::raii::CommandPool(device, poolInfo);
}

void VulkanCommandPool::Reset(vk::CommandPoolResetFlags flags)
{
    m_CommandPool.reset(flags);
}

void VulkanCommandPool::CleanUp()
{
    m_CommandPool.clear();
}
} // namespace UHE::RHI::VULKAN
