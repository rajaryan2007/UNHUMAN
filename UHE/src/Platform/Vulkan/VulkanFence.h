#pragma once

#include <cstdint>
#include <vulkan/vulkan_raii.hpp>

namespace UHE::RHI::VULKAN
{
class VulkanContext;

class VulkanFence
{
public:
    VulkanFence() = default;
    ~VulkanFence() = default;
    VulkanFence(VulkanFence&) = delete;
    VulkanFence operator=(VulkanFence) = delete;

    void Init(bool signaled = false, VulkanContext* context = nullptr);
    void ShutDown();

    void Wait(u64 timeout = UINT64_MAX);
    void Reset();
    bool IsSignaled();

    inline vk::Fence GetHandle() const;

private:
    VulkanContext* m_context = nullptr;
    vk::raii::Fence m_Fence = nullptr;
};
} // namespace UHE::RHI::VULKAN
