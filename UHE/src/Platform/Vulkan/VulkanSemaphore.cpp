#include "VulkanSemaphore.h"
#include "VulkanContext.h"
#include "VulkanExtensionCheck.h"

namespace UHE::RHI::VULKAN
{
  void VulkanSemaphore::Init(bool requestTimeline, u64 intialValue, VulkanContext* context)
  {
    ctx = context;
    
    m_IsTimeline = requestTimeline && ctx->CheckExtensions->Supports(Extension::TimelineSemaphore);

    vk::SemaphoreTypeCreateInfo timelineInfo;
    timelineInfo.semaphoreType = m_IsTimeline ? vk::SemaphoreType::eTimeline : vk::SemaphoreType::eBinary;
    timelineInfo.initialValue = intialValue;

    vk::SemaphoreCreateInfo createInfo;
    createInfo.pNext = &timelineInfo;

    m_Semaphore = vk::raii::Semaphore(*ctx->logicalDeviceHandle, createInfo);
  }

    void VulkanSemaphore::ShutDown(){
       
    }

    void VulkanSemaphore::WaitCPU(u64 value)
    {
        if (!m_IsTimeline)
        return;
    
        vk::SemaphoreWaitInfo waitInfo;
        waitInfo.semaphoreCount = 1;
        waitInfo.pSemaphores = &m_Semaphore.operator*();
        waitInfo.pValues = &value;
    
        const vk::Result waitResult = ctx->logicalDeviceHandle->waitSemaphores(waitInfo, UINT64_MAX);
        (void)waitResult;
    }

    u64 VulkanSemaphore::GetValue()
    {
        if (!m_IsTimeline)
            return 0;
    
        return m_Semaphore.getCounterValue();
    }

    
} // namespace UHE::RHI::VULKAN
