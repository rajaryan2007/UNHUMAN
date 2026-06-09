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

VulkanDevice::VulkanDevice(const SwapchainDesc& swapDesc)
{
    m_WindowHandle = static_cast<GLFWwindow*>(swapDesc.nativeWindow);
    m_WindowWidth = swapDesc.width;
    m_WindowHeight = swapDesc.height;
    InitVulkan(swapDesc);
}

VulkanDevice::~VulkanDevice()
{
    CleanupVulkan();
}

void VulkanDevice::InitVulkan(const SwapchainDesc& swapDesc)
{
    UHE_PROFILE_FUNCTION();

    m_Instance.initialize();
    m_LogicalDevice.CreateSurface(m_Instance, m_WindowHandle);
    m_PhysicalDevice.initPhysicalDevice(m_Instance);

    m_LogicalDevice.initialize(m_PhysicalDevice, *m_LogicalDevice.getSurface(), m_Instance);
    m_Allocator = m_LogicalDevice.getAllocator();

    m_SwapChain.createSwapChain(m_LogicalDevice.getLogicalDevice(), m_PhysicalDevice.getPhysicalDevice(),
                                m_LogicalDevice.getSurface(), m_WindowHandle);

    for (auto& frame : m_Frames)
    {
        frame.Init(m_LogicalDevice.getLogicalDevice(), m_LogicalDevice.getGraphicsQueueFamilyIndex());
    }

    m_RenderFinishedSemaphores.clear();
    for (size_t i = 0; i < m_SwapChain.GetImages().size(); i++)
    {
        vk::SemaphoreCreateInfo semaphoreInfo{};
        m_RenderFinishedSemaphores.emplace_back(m_LogicalDevice.getLogicalDevice(), semaphoreInfo);
    }

    vk::CommandPoolCreateInfo uploadPoolInfo{};
    uploadPoolInfo.queueFamilyIndex = m_LogicalDevice.getGraphicsQueueFamilyIndex();
    m_UploadCommandPool = vk::raii::CommandPool(m_LogicalDevice.getLogicalDevice(), uploadPoolInfo);

    vk::FenceCreateInfo fenceInfo{};
    m_UploadFence = vk::raii::Fence(m_LogicalDevice.getLogicalDevice(), fenceInfo);

    m_DescriptorManager.init(*this);

    for (auto& frame : m_Frames)
    {
        frame.GetCommandBuffer().SetContext(&m_LogicalDevice.getLogicalDevice(), &m_DescriptorManager);
    }

    UHE_CORE_INFO("Vulkan device initialized successfully");
}

void VulkanDevice::CleanupVulkan()
{
    WaitIdle();

    m_DescriptorManager.cleanup();

    for (auto& frame : m_Frames)
    {
        frame.Cleanup();
    }

    m_UploadFence = nullptr;
    m_UploadCommandPool = nullptr;
    m_RenderFinishedSemaphores.clear();

    m_SwapChain.cleanupSwapChain();
    m_LogicalDevice.cleanup();
}

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
    m_SwapChain.createSwapChain(m_LogicalDevice.getLogicalDevice(), m_PhysicalDevice.getPhysicalDevice(),
                                m_LogicalDevice.getSurface(), m_WindowHandle);

    m_RenderFinishedSemaphores.clear();
    for (size_t i = 0; i < m_SwapChain.GetImages().size(); i++)
    {
        vk::SemaphoreCreateInfo semaphoreInfo{};
        m_RenderFinishedSemaphores.emplace_back(m_LogicalDevice.getLogicalDevice(), semaphoreInfo);
    }
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
    texture->Init(*this, desc);
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

void VulkanDevice::DestroyBuffer(BufferHandle handle)
{
    if (handle)
    {
        auto* buffer = reinterpret_cast<VulkanBuffer*>(handle);
        delete buffer;
    }
}

void VulkanDevice::DestroyTexture(TextureHandle handle)
{
    if (handle)
    {
        auto* texture = reinterpret_cast<VulkanTexture*>(handle);
        delete texture;
    }
}

void VulkanDevice::DestroyShader(ShaderHandle handle)
{
    if (handle)
    {
        auto* shader = reinterpret_cast<VulkanShader*>(handle);
        delete shader;
    }
}

void VulkanDevice::DestroyGraphicsPipeline(PipelineHandle handle)
{
    if (handle)
    {
        auto* pipeline = reinterpret_cast<VulkanGraphicPipeline*>(handle);
        delete pipeline;
    }
}

void VulkanDevice::Begin()
{
    auto waitResult = m_LogicalDevice.getLogicalDevice().waitForFences({*m_Frames[m_CurrentFrame].GetInFlightFence()},
                                                                       VK_TRUE, UINT64_MAX);

    m_LogicalDevice.getLogicalDevice().resetFences({*m_Frames[m_CurrentFrame].GetInFlightFence()});

    auto acquireResult = m_SwapChain.GetSwapchain().acquireNextImage(
        UINT64_MAX, *m_Frames[m_CurrentFrame].GetimageAvailableSemaphore(), nullptr);

    if (acquireResult.result == vk::Result::eErrorOutOfDateKHR)
    {
        RecreateSwapchain();
        acquireResult = m_SwapChain.GetSwapchain().acquireNextImage(
            UINT64_MAX, *m_Frames[m_CurrentFrame].GetimageAvailableSemaphore(), nullptr);
    }
    else if (acquireResult.result != vk::Result::eSuccess && acquireResult.result != vk::Result::eSuboptimalKHR)
    {
        throw std::runtime_error("Failed to acquire swap chain image!");
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
    submitInfo.pSignalSemaphores = &(*m_RenderFinishedSemaphores[m_ImageIndex]);

    vk::raii::Queue& m_graphicsQueue = m_LogicalDevice.getGraphicsQueue();
    m_graphicsQueue.submit(submitInfo, *m_Frames[m_CurrentFrame].GetInFlightFence());

    vk::PresentInfoKHR presentInfo{};
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &(*m_RenderFinishedSemaphores[m_ImageIndex]);
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

    vk::raii::CommandBuffers cmdBuffers(m_LogicalDevice.getLogicalDevice(), allocInfo);
    vk::raii::CommandBuffer cmd = std::move(cmdBuffers[0]);

    vk::CommandBufferBeginInfo beginInfo{};
    beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
    cmd.begin(beginInfo);

    function(cmd);

    cmd.end();

    vk::SubmitInfo submitInfo{};
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &(*cmd);

    vk::raii::Queue& m_graphicsQueue = m_LogicalDevice.getGraphicsQueue();
    m_graphicsQueue.submit(submitInfo, *m_UploadFence);

    // Wait for the command to finish executing
    auto waitResult = m_LogicalDevice.getLogicalDevice().waitForFences({*m_UploadFence}, VK_TRUE, UINT64_MAX);
    UHE_CORE_ASSERT(waitResult == vk::Result::eSuccess, "Failed to wait for upload fence!");

    m_LogicalDevice.getLogicalDevice().resetFences({*m_UploadFence});
    m_UploadCommandPool.reset();
}

void VulkanDevice::WaitIdle()
{
    getLogicalDevClass().getLogicalDevice().waitIdle();
}

} // namespace UHE::RHI::VULKAN
