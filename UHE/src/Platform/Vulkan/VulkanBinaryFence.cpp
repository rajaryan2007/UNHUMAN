#include "uhepch.h"
#include "VulkanBinaryFence.h"
#include "VulkanContext.h"

namespace UHE::RHI::VULKAN
{
void VulkanBinaryFence::Init(VulkanContext* context, bool signaled)
{
    m_context = context;

    vk::FenceCreateInfo info{};
    if (signaled)
    {
        info.flags = vk::FenceCreateFlagBits::eSignaled;
    }

    // RAII Magic: This automatically calls vkCreateFence!
    m_Fence = vk::raii::Fence(*m_context->logicalDeviceHandle, info);
}

void VulkanBinaryFence::Shutdown()
{
    // RAII Magic: Setting to nullptr automatically calls vkDestroyFence!
    m_Fence = nullptr;
    m_context = nullptr;
}

void VulkanBinaryFence::WaitOnCpuIn(const SemaphoneSubmitInfo& submitInfo) const
{
    if (m_context && *m_Fence)
    {
        // Wait for the fence to be signaled by the GPU
        (void)m_context->logicalDeviceHandle->waitForFences({*m_Fence}, VK_TRUE, UINT64_MAX);
    }
}

void VulkanBinaryFence::ResetIn() const
{
    if (m_context && *m_Fence)
    {
        m_context->logicalDeviceHandle->resetFences({*m_Fence});
    }
}

bool VulkanBinaryFence::IsSignaled() const
{
    if (!m_context || !*m_Fence)
        return false;
    return m_context->logicalDeviceHandle->getFenceStatus(*m_Fence) == vk::Result::eSuccess;
}

} // namespace UHE::RHI::VULKAN
