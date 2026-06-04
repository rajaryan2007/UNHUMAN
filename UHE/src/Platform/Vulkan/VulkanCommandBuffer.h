#pragma once
#include <vulkan/vulkan_raii.hpp>
// #include "VulkanCommandPool.h"

namespace UHE::RHI::VULKAN
{
class VulkanDevice;
class VulkanCommandPool;
class VulkanCommandBuffer
{
public:
    VulkanCommandBuffer() = default;
    ~VulkanCommandBuffer() = default;

    VulkanCommandBuffer(const VulkanCommandBuffer&) = delete;
    VulkanCommandBuffer& operator=(const VulkanCommandBuffer&) = delete;

    VulkanCommandBuffer(VulkanCommandBuffer&&) = default;
    VulkanCommandBuffer& operator=(VulkanCommandBuffer&&) = default;

    void Allocate(vk::raii::Device& device, VulkanCommandPool& pool, bool isPrimary = true);
    void Free();
    void BeginCommandBuffer(vk::CommandBufferUsageFlags flags = {});
    void Reset(vk::CommandBufferUsageFlags flags = {});
    void EndCommandBuffer();

    inline const vk::raii::CommandBuffer& GetHandle() const { return m_CommandBuffer; }
    inline vk::raii::CommandBuffer& GetHandle() { return m_CommandBuffer; }

private:
    vk::raii::CommandBuffer m_CommandBuffer{nullptr};
};
} // namespace UHE::RHI::VULKAN
