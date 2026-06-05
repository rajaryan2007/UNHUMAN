#include "uhepch.h"
#include "VulkanCommandBuffer.h"
#include <vulkan/vulkan_core.h>
#include "VulkanCommandPool.h"
#include "vulkan/vulkan.hpp"

namespace UHE::RHI::VULKAN
{
void VulkanCommandBuffer::Allocate(vk::raii::Device& device, VulkanCommandPool& pool, bool isPrimary)
{
    vk::CommandBufferAllocateInfo allocInfo{};
    allocInfo.commandPool = *pool.GetHandle();
    allocInfo.level = isPrimary ? vk::CommandBufferLevel::ePrimary : vk::CommandBufferLevel::eSecondary;
    allocInfo.commandBufferCount = 1;

    vk::raii::CommandBuffers cmdBuffers(device, allocInfo);
    m_CommandBuffer = std::move(cmdBuffers[0]);
}

void VulkanCommandBuffer::Free()
{
    m_CommandBuffer.clear();
}

void VulkanCommandBuffer::BeginCommandBuffer(vk::CommandBufferUsageFlags flags)
{
    vk::CommandBufferBeginInfo beginInfo{};
    beginInfo.flags = flags;
    m_CommandBuffer.begin(beginInfo);
}

void VulkanCommandBuffer::Reset(vk::CommandBufferUsageFlags flags)
{
    vkResetCommandBuffer(*m_CommandBuffer, static_cast<VkCommandBufferResetFlags>(flags));
}

void VulkanCommandBuffer::EndCommandBuffer()
{
    m_CommandBuffer.end();
}
} // namespace UHE::RHI::VULKAN
