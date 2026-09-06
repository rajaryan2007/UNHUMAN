#include "VulkanSemaphore.h"
#include "VulkanContext.h"


namespace UHE::RHI::VULKAN
{
  void VulkanSemaphore::Init(bool requestTimeline, u64 intialValue, VulkanContext* context)
  {
    ctx = context;
    
    m_IsTimeline = requestTimeline && ctx->CheckExtensions->IsEnable("VK_KHR_timeline_semaphore");

    vk::SemaphoreTypeCreateInfo timelineInfo;
    timelineInfo.semaphoreType = m_IsTimeline ? vk::SemaphoreType::eTimeline : vk::SemaphoreType::eBinary;
    timelineInfo.initialValue = intialValue;

    vk::SemaphoreCreateInfo createInfo;
    createInfo.pNext = &timelineInfo;

    m_Semaphore = vk::raii::Semaphore(ctx->GetDevice(), createInfo);
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
    
        ctx->GetDevice().waitSemaphores(waitInfo, UINT64_MAX);
    }

    u64 VulkanSemaphore::GetValue()
    {
        if (!m_IsTimeline)
            return 0;
    
        return ctx->GetDevice().getSemaphoreCounterValue(*m_Semaphore);
    }

    
} // namespace UHE::RHI::VULKAN
