#pragma once

#include <cstdint>
#include <vulkan/vulkan_raii.hpp>

namespace UHE::RHI::VULKAN
{
class VulkanFence
{
public:
    VulkanFence() = default;
    ~VulkanFence() = default;
    VulkanFence(VulkanFence&) = delete;
    VulkanFence operator=(VulkanFence) = delete;

    void Init();
    void ShutDown();

    void Wait(u64 timeout = UINT64_MAX);
    void Reset();
    bool IsSinaled();

    inline vk::Fence GetHandle() const;

private:
    vk::raii::Fence m_Fence = nullptr;
};
} // namespace UHE::RHI::VULKAN
