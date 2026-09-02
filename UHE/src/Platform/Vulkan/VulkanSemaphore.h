#pragma once
#include <vulkan/vulkan_raii.hpp>
#include "vulkan/vulkan.hpp"

namespace UHE::RHI::VULKAN
{
class VulkanContext;

class VulkanSemaphore
{
public:
    VulkanSemaphore() = default;
    ~VulkanSemaphore() = default;
    VulkanSemaphore(VulkanSemaphore&) = delete;
    VulkanSemaphore operator=(VulkanSemaphore&) = delete;

    void Init(bool requestTimeline = false, u64 intialValue = 0, VulkanContext* context = nullptr);
    void ShutDown();
    void WaitCPU(u64 value);
    u64 GetValue();

    inline vk::Semaphore GetHandle() const;
    inline bool inTimeline() const { return m_IsTimeline; }

private:
    bool m_IsTimeline = false; // true only if request AND hardware
    VulkanContext* ctx = nullptr;
    vk::raii::Semaphore m_Semaphore = nullptr;

    // TODO:  fallback state for emulating timeline using multiple binary semaphore
};

struct SemaphoneSubmitInfo
{
    VulkanSemaphore* semaphore;
    vk::PipelineStageFlags2 stageMask;
};

struct SubmitDesc
{
    std::vector<SemaphoneSubmitInfo> waitSemaphores;
    std::vector<vk::CommandBuffer> commandBuffers;
    std::vector<SemaphoneSubmitInfo> signalSemaphores;
};

} // namespace UHE::RHI::VULKAN
