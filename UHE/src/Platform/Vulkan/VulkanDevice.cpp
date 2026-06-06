#include "uhepch.h"
#include "VulkanDevice.h"
#include <GLFW/glfw3.h>
#include <atomic>
#include <common/TracyQueue.hpp>
#include <cstdint>
#include <unistd.h>
#include <vulkan/vulkan_raii.hpp>
#include "Platform/Vulkan/VulkanBuffer.h"
#include "Platform/Vulkan/VulkanGraphicPipeline.h"
#include "Platform/Vulkan/VulkanShader.h"
#include "Platform/Vulkan/VulkanTexture.h"
#include "UHE/Core/Log.h"
#include "UHE/RHI/RHITypes.h"

namespace UHE::RHI::VULKAN
{

void VulkanDevice::RecreateSwapchain()
{
    UHE_PROFILE_FUNCTION();

    int width = 0, height = 0;
    glfwGetFramebufferSize(m_WindowHandle, &width, &height);
    while (width == 0 || height == 0)
    {
        glfwGetFramebufferSize(m_WindowHandle, &width, &height);
        glfwWaitEvents();
    }

    WaitIdle();
    m_SwapChain.cleanupSwapChain();
    m_SwapChain.createSwapChain(m_LogDevice, m_PhysicalDevice.getPhysicalDevice(), m_Surface, m_WindowHandle);
}

// ─── Resource Management Stubs  ───

BufferHandle VulkanDevice::CreateBuffer(const BufferDesc& desc)
{
    VulkanBuffer* buffer = new VulkanBuffer();

    vk::BufferUsageFlags usage{};
    if (desc.usage == BufferUsage::Vertex)
    {
        usage = vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst;
    }
    else if (desc.usage == BufferUsage::Index)
    {
        usage = vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst;
    }
    else if (desc.usage == BufferUsage::Uniform)
    {
        usage = vk::BufferUsageFlagBits::eUniformBuffer;
    }
    else if (desc.usage == BufferUsage::Storage)
    {
        usage = vk::BufferUsageFlagBits::eStorageBuffer;
    }
    else if (desc.usage == BufferUsage::Staging)
    {
        usage = vk::BufferUsageFlagBits::eTransferDst;
    }

    VmaMemoryUsage memUsage = desc.hostVisible ? VMA_MEMORY_USAGE_CPU_TO_GPU : VMA_MEMORY_USAGE_GPU_ONLY;
    buffer->init(m_Allocator, desc.size, usage, memUsage);
    return reinterpret_cast<BufferHandle>(buffer);
}
TextureHandle VulkanDevice::CreateTexture(const TextureDesc& desc)
{
    VulkanTexture* texture = new VulkanTexture();
    texture->Init(*this);
    return reinterpret_cast<TextureHandle>(texture);
}
ShaderHandle VulkanDevice::CreateShader(const ShaderDesc& desc)
{
    VulkanShader* shader = new VulkanShader();
    shader->Create(m_LogicalDevice.getLogicalDevice(), desc);
    return reinterpret_cast<ShaderHandle>(shader);
}
PipelineHandle VulkanDevice::CreateGraphicsPipeline(const GraphicsPipelineDesc& desc)
{
    VulkanGraphicPipeline* pipeline = new VulkanGraphicPipeline();
    pipeline->createGraphicsPipeline(m_LogicalDevice, m_DescriptorManager, desc);
    return reinterpret_cast<PipelineHandle>(pipeline);
}

void VulkanDevice::Begin()
{
    auto waitResult = m_LogDevice.waitForFences({*m_Frames[m_CurrentFrame].GetInFlightFence()}, VK_TRUE, UINT64_MAX);

    m_LogDevice.resetFences({*m_Frames[m_CurrentFrame].GetInFlightFence()});

    auto acquireResult = m_SwapChain.GetSwapchain().acquireNextImage(
        UINT64_MAX, *m_Frames[m_CurrentFrame].GetimageAvailableSemaphore(), nullptr);

    if (acquireResult.result == vk::Result::eErrorOutOfDateKHR || acquireResult.result == vk::Result::eSuboptimalKHR)
    {
        RecreateSwapchain();
        return;
    }

    m_ImageIndex = acquireResult.value;

    m_Frames[m_CurrentFrame].GetDeletionQueue().Flush();
    m_Frames[m_CurrentFrame].GetCommandBuffer().Reset();

    m_Frames[m_CurrentFrame].GetCommandBuffer().BeginCommandBuffer(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
}

void VulkanDevice::End()
{
    vk::raii::CommandBuffer& cmd = m_Frames[m_CurrentFrame].GetCommandBuffer().GetHandle();
    cmd.end();

    vk::SubmitInfo submitInfo{};
    vk::PipelineStageFlags waitResult[] = {vk::PipelineStageFlagBits::eColorAttachmentOutput};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &(*m_Frames[m_CurrentFrame].GetimageAvailableSemaphore());
    submitInfo.pWaitDstStageMask = waitResult;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &(*cmd);
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &(*m_Frames[m_CurrentFrame].GetrenderFinishedSemaphore());

    m_graphicsQueue.submit(submitInfo, *m_Frames[m_CurrentFrame].GetInFlightFence());

    vk::PresentInfoKHR presentInfo{};
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &(*m_Frames[m_CurrentFrame].GetrenderFinishedSemaphore());
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &(*m_SwapChain.GetSwapchain());
    presentInfo.pImageIndices = &m_ImageIndex;

    try
    {
        auto presentResult = m_graphicsQueue.presentKHR(presentInfo);
        if (presentResult == vk::Result::eErrorOutOfDateKHR || presentResult == vk::Result::eSuboptimalKHR ||
            m_FramebufferResized)
        {
            m_FramebufferResized = false;
            RecreateSwapchain();
        }
    }
    catch (vk::OutOfDateKHRError&)
    {
        m_FramebufferResized = false;
        RecreateSwapchain();
    }
    m_CurrentFrame = (m_CurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

RHICommandBuffer& VulkanDevice::GetCurrentCommandBuffer()
{
    return m_Frames[m_CurrentFrame].GetCommandBuffer();
}

void VulkanDevice::ImmediateSubmit(std::function<void(vk::raii::CommandBuffer& cmd)>&& function)
{
    vk::CommandBufferAllocateInfo allocInfo{};
    allocInfo.commandPool = *m_UploadCommandPool;
    allocInfo.level = vk::CommandBufferLevel::ePrimary;
    allocInfo.commandBufferCount = 1;

    vk::raii::CommandBuffers cmdBuffers(m_LogDevice, allocInfo);
    vk::raii::CommandBuffer cmd = std::move(cmdBuffers[0]);

    vk::CommandBufferBeginInfo beginInfo{};
    beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
    cmd.begin(beginInfo);

    function(cmd);

    cmd.end();

    vk::SubmitInfo submitInfo{};
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &(*cmd);

    m_graphicsQueue.submit(submitInfo, *m_UploadFence);

    // Wait for the command to finish executing
    auto waitResult = m_LogDevice.waitForFences({*m_UploadFence}, VK_TRUE, UINT64_MAX);
    VG_CORE_ASSERT(waitResult == vk::Result::eSuccess, "Failed to wait for upload fence!");

    m_LogDevice.resetFences({*m_UploadFence});
    m_UploadCommandPool.reset();
}

} // namespace UHE::RHI::VULKAN
