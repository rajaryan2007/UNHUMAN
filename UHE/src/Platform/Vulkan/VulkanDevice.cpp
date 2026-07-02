#include "uhepch.h"
#include "VulkanDevice.h"
#include <GLFW/glfw3.h>
#include <atomic>
#include <common/TracyQueue.hpp>
#include <cstdint>
#ifdef _WIN32
// Windows-specific includes if needed
#else
    #include <unistd.h>
#endif
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
        vk::SemaphoreCreateInfo semaphoreInfo{.flags = {}};
        m_RenderFinishedSemaphores.emplace_back(m_LogicalDevice.getLogicalDevice(), semaphoreInfo);
    }

    vk::CommandPoolCreateInfo uploadPoolInfo{.flags = {},
                                             .queueFamilyIndex = m_LogicalDevice.getGraphicsQueueFamilyIndex()};
    m_UploadCommandPool = vk::raii::CommandPool(m_LogicalDevice.getLogicalDevice(), uploadPoolInfo);

    vk::FenceCreateInfo fenceInfo{.flags = {}};
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
        vk::SemaphoreCreateInfo semaphoreInfo{.flags = {}};
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
        usage = vk::BufferUsageFlagBits::eTransferSrc;
    }

    VmaMemoryUsage memUsage = desc.hostVisible ? VMA_MEMORY_USAGE_CPU_TO_GPU : VMA_MEMORY_USAGE_GPU_ONLY;
    buffer->init(m_Allocator, desc.size, usage, memUsage);
    return reinterpret_cast<BufferHandle>(buffer);
}

u32 VulkanDevice::GetBufferBindlessIndex(BufferHandle handle)
{
    if (!handle)
        return 0;
    auto* buffer = reinterpret_cast<VulkanBuffer*>(handle);
    return m_DescriptorManager.RegisterBuffer(m_LogicalDevice.getLogicalDevice(), buffer->GetHandle(),
                                              buffer->GetSize());
}

TextureHandle VulkanDevice::CreateTexture(const TextureDesc& desc)
{
    auto* texture = new VulkanTexture();
    texture->Init(*this, desc);
    return reinterpret_cast<TextureHandle>(texture);
}
ShaderHandle VulkanDevice::CreateShader(const ShaderDesc& desc)
{
    auto* shader = new VulkanShader();
    shader->Create(m_LogicalDevice.getLogicalDevice(), desc);
    return reinterpret_cast<ShaderHandle>(shader);
}
PipelineHandle VulkanDevice::CreateGraphicsPipeline(const GraphicsPipelineDesc& desc)
{
    auto* pipeline = new VulkanGraphicPipeline();
    pipeline->createGraphicsPipeline(m_LogicalDevice, m_DescriptorManager, desc);
    return reinterpret_cast<PipelineHandle>(pipeline);
}

void VulkanDevice::DestroyBuffer(BufferHandle handle)
{
    if (handle)
    {
        auto* buffer = reinterpret_cast<VulkanBuffer*>(handle);
        m_Frames[m_CurrentFrame].GetDeletionQueue().Push([buffer]() { delete buffer; });
    }
}

void VulkanDevice::DestroyTexture(TextureHandle handle)
{
    if (handle)
    {
        auto* texture = reinterpret_cast<VulkanTexture*>(handle);
        m_Frames[m_CurrentFrame].GetDeletionQueue().Push([texture]() { delete texture; });
    }
}

void VulkanDevice::DestroyShader(ShaderHandle handle)
{
    if (handle)
    {
        auto* shader = reinterpret_cast<VulkanShader*>(handle);
        m_Frames[m_CurrentFrame].GetDeletionQueue().Push([shader]() { delete shader; });
    }
}

void VulkanDevice::DestroyGraphicsPipeline(PipelineHandle handle)
{
    if (handle)
    {
        auto* pipeline = reinterpret_cast<VulkanGraphicPipeline*>(handle);
        m_Frames[m_CurrentFrame].GetDeletionQueue().Push([pipeline]() { delete pipeline; });
    }
}

void VulkanDevice::Begin()
{
    auto waitResult = m_LogicalDevice.getLogicalDevice().waitForFences({*m_Frames[m_CurrentFrame].GetInFlightFence()},
                                                                       VK_TRUE, UINT64_MAX);

    m_LogicalDevice.getLogicalDevice().resetFences({*m_Frames[m_CurrentFrame].GetInFlightFence()});

    vk::Result acquireResult = vk::Result::eSuccess;
    uint32_t imageIndex = 0;

    try
    {
        auto [res, idx] = m_SwapChain.GetSwapchain().acquireNextImage(
            UINT64_MAX, *m_Frames[m_CurrentFrame].GetimageAvailableSemaphore(), nullptr);
        acquireResult = res;
        imageIndex = idx;
    }
    catch (const vk::OutOfDateKHRError&)
    {
        acquireResult = vk::Result::eErrorOutOfDateKHR;
    }

    if (acquireResult == vk::Result::eErrorOutOfDateKHR)
    {
        RecreateSwapchain();
        try
        {
            auto [res, idx] = m_SwapChain.GetSwapchain().acquireNextImage(
                UINT64_MAX, *m_Frames[m_CurrentFrame].GetimageAvailableSemaphore(), nullptr);
            acquireResult = res;
            imageIndex = idx;
        }
        catch (const std::exception&)
        {
            throw std::runtime_error("Failed to acquire swap chain image!");
        }
    }
    else if (acquireResult != vk::Result::eSuccess && acquireResult != vk::Result::eSuboptimalKHR)
    {
        throw std::runtime_error("Failed to acquire swap chain image!");
    }

    m_ImageIndex = imageIndex;

    m_Frames[m_CurrentFrame].GetDeletionQueue().Flush();
    m_Frames[m_CurrentFrame].GetCommandBuffer().Reset();

    m_Frames[m_CurrentFrame].GetCommandBuffer().BeginCommandBuffer(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
}

void VulkanDevice::End()
{
    vk::raii::CommandBuffer& cmd = m_Frames[m_CurrentFrame].GetCommandBuffer().GetHandle();
    cmd.end();

    vk::PipelineStageFlags waitResult[] = {vk::PipelineStageFlagBits::eColorAttachmentOutput};
    vk::SubmitInfo submitInfo{.waitSemaphoreCount = 1,
                              .pWaitSemaphores = &(*m_Frames[m_CurrentFrame].GetimageAvailableSemaphore()),
                              .pWaitDstStageMask = waitResult,
                              .commandBufferCount = 1,
                              .pCommandBuffers = &(*cmd),
                              .signalSemaphoreCount = 1,
                              .pSignalSemaphores = &(*m_RenderFinishedSemaphores[m_ImageIndex])};

    vk::raii::Queue& m_graphicsQueue = m_LogicalDevice.getGraphicsQueue();
    m_graphicsQueue.submit(submitInfo, *m_Frames[m_CurrentFrame].GetInFlightFence());

    vk::PresentInfoKHR presentInfo{.waitSemaphoreCount = 1,
                                   .pWaitSemaphores = &(*m_RenderFinishedSemaphores[m_ImageIndex]),
                                   .swapchainCount = 1,
                                   .pSwapchains = &(*m_SwapChain.GetSwapchain()),
                                   .pImageIndices = &m_ImageIndex,
                                   .pResults = nullptr};

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
    vk::CommandBufferAllocateInfo allocInfo{
        .commandPool = *m_UploadCommandPool, .level = vk::CommandBufferLevel::ePrimary, .commandBufferCount = 1};

    vk::raii::CommandBuffers cmdBuffers(m_LogicalDevice.getLogicalDevice(), allocInfo);
    vk::raii::CommandBuffer cmd = std::move(cmdBuffers[0]);

    vk::CommandBufferBeginInfo beginInfo{.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit};
    cmd.begin(beginInfo);

    function(cmd);

    cmd.end();

    vk::SubmitInfo submitInfo{.commandBufferCount = 1, .pCommandBuffers = &(*cmd)};

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

void VulkanDevice::ResetCommandBuffers()
{
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        m_Frames[i].GetCommandBuffer().Reset();
    }
}

void VulkanDevice::ReadPixel(TextureHandle handle, int x, int y, void* outData)
{
    auto* texture = reinterpret_cast<VulkanTexture*>(handle);
    vk::Image image = texture->GetImage();

    // 1. Create a CPU-visible buffer
    VkBuffer stagingBuffer;
    VmaAllocation stagingAlloc;

    VkBufferCreateInfo bufferInfo{.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
                                  .pNext = nullptr,
                                  .flags = 0,
                                  .size = 4, // Read 4 bytes
                                  .usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                  .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
                                  .queueFamilyIndexCount = 0,
                                  .pQueueFamilyIndices = nullptr};

    VmaAllocationCreateInfo allocInfo{.flags = 0,
                                      .usage = VMA_MEMORY_USAGE_GPU_TO_CPU,
                                      .requiredFlags = 0,
                                      .preferredFlags = 0,
                                      .memoryTypeBits = 0,
                                      .pool = VK_NULL_HANDLE,
                                      .pUserData = nullptr,
                                      .priority = 0.0f};

    VkResult res = vmaCreateBuffer(m_Allocator, &bufferInfo, &allocInfo, &stagingBuffer, &stagingAlloc, nullptr);
    if (res != VK_SUCCESS)
    {
        UHE_CORE_ERROR("Failed to create staging buffer for ReadPixel!");
        return;
    }

    // 2. Allocate and begin command buffer
    vk::CommandBufferAllocateInfo allocInfoCmd{
        .commandPool = {}, // Note: immediate submit overrides this inside, but we provide it here if needed or let
                           // default init. Wait, ImmediateSubmit creates its own pool? Yes, ImmediateSubmit doesn't
                           // take allocInfoCmd. It's unused!
        .level = vk::CommandBufferLevel::ePrimary,
        .commandBufferCount = 1};
    ImmediateSubmit(
        [&](vk::raii::CommandBuffer& cmd)
        {
            // 3. Transition image layout to TRANSFER_SRC_OPTIMAL
            vk::ImageMemoryBarrier barrier{.srcAccessMask = vk::AccessFlagBits::eMemoryRead,
                                           .dstAccessMask = vk::AccessFlagBits::eTransferRead,
                                           .oldLayout = vk::ImageLayout::eShaderReadOnlyOptimal,
                                           .newLayout = vk::ImageLayout::eTransferSrcOptimal,
                                           .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                                           .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                                           .image = image,
                                           .subresourceRange = {.aspectMask = vk::ImageAspectFlagBits::eColor,
                                                                .baseMipLevel = 0,
                                                                .levelCount = 1,
                                                                .baseArrayLayer = 0,
                                                                .layerCount = 1}};

            cmd.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eTransfer,
                                vk::DependencyFlags{}, nullptr, nullptr, barrier);

            // 4. Copy image to buffer
            vk::BufferImageCopy region{.bufferOffset = 0,
                                       .bufferRowLength = 0,
                                       .bufferImageHeight = 0,
                                       .imageSubresource = {.aspectMask = vk::ImageAspectFlagBits::eColor,
                                                            .mipLevel = 0,
                                                            .baseArrayLayer = 0,
                                                            .layerCount = 1},
                                       .imageOffset = vk::Offset3D{x, y, 0},
                                       .imageExtent = vk::Extent3D{1, 1, 1}};

            cmd.copyImageToBuffer(image, vk::ImageLayout::eTransferSrcOptimal, stagingBuffer, region);

            // 5. Transition image layout back
            barrier.oldLayout = vk::ImageLayout::eTransferSrcOptimal;
            barrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
            barrier.srcAccessMask = vk::AccessFlagBits::eTransferRead;
            barrier.dstAccessMask = vk::AccessFlagBits::eMemoryRead;

            cmd.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eTransfer,
                                vk::DependencyFlags{}, nullptr, nullptr, barrier);
        });

    // 7. Map memory and read
    void* mappedData = nullptr;
    if (vmaMapMemory(m_Allocator, stagingAlloc, &mappedData) == VK_SUCCESS && mappedData)
    {
        memcpy(outData, mappedData, 4);
        vmaUnmapMemory(m_Allocator, stagingAlloc);
    }

    vmaDestroyBuffer(m_Allocator, stagingBuffer, stagingAlloc);
}

void VulkanDevice::SetVulkanObjectName(uint64_t objectHandle, vk::ObjectType objectType, const char* name)
{
    vk::DebugUtilsObjectNameInfoEXT nameInfo{
        .objectType = objectType, .objectHandle = objectHandle, .pObjectName = name};
    m_LogicalDevice.getLogicalDevice().setDebugUtilsObjectNameEXT(nameInfo);
}

} // namespace UHE::RHI::VULKAN
