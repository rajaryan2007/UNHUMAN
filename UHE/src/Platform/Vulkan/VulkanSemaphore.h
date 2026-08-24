#pragma once
#include <vulkan/vulkan_raii.hpp>

namespace UHE::RHI::VULKAN
{
class VulkanSemaphore
{
public:
    VulkanSemaphore() = default;
    ~VulkanSemaphore() = default;
    VulkanSemaphore(VulkanSemaphore&) = delete;
    VulkanSemaphore operator=(VulkanSemaphore&) = delete;

    void Init();
    void ShutDown();

private:
    vk::raii::Semaphore m_Semaphore = nullptr;
};

} // namespace UHE::RHI::VULKAN
