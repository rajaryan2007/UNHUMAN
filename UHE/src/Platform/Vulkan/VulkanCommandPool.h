#pragma once

#include <vulkan/vulkan_raii.hpp>
namespace UHE::RHI::VULKAN
{
class VulkanCommandPool
{
public:
    VulkanCommandPool() = default;
    ~VulkanCommandPool() = default;
    VulkanCommandPool(const VulkanCommandPool&) = delete;
    VulkanCommandPool& operator=(const VulkanCommandPool&) = delete;

    void Init(vk::raii::Device& device, u32 queueFamilyIndex, vk::CommandPoolCreateFlags flags = {});
    void CleanUp();
    void Reset(vk::CommandPoolResetFlags flags = {});
    inline const vk::raii::CommandPool& GetHandle() const { return m_CommandPool; }
    inline vk::raii::CommandPool& GetHandle() { return m_CommandPool; }

private:
    vk::raii::CommandPool m_CommandPool{nullptr};
};
}; // namespace UHE::RHI::VULKAN
