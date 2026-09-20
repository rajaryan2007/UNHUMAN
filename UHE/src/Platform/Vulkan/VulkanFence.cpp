#include "uhepch.h"
#include "VulkanFence.h"
#include "VulkanContext.h"

namespace UHE::RHI::VULKAN
{

void VulkanFence::Init(bool signaled, VulkanContext* context)
{
    m_context = context;
}

void VulkanFence::Wait(u64 timeout) {}

void VulkanFence::Reset() {}

bool VulkanFence::IsSignaled() {

};

void VulkanFence::ShutDown()
{
    m_context = nullptr;
};
} // namespace UHE::RHI::VULKAN
