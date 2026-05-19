#include "uhepch.h"
#include "VulkanDevice.h"
#include "UHE/Core/Log.h"
#include <GLFW/glfw3.h>

namespace UHE::RHI {

VulkanDevice::VulkanDevice(const SwapchainDesc& swapDesc) 
    : m_WindowHandle(static_cast<GLFWwindow*>(swapDesc.nativeWindow)),
      m_WindowWidth(swapDesc.width),
      m_WindowHeight(swapDesc.height)
{
    InitVulkan(swapDesc);
}

VulkanDevice::~VulkanDevice() {
    WaitIdle();
    CleanupVulkan();
}

void VulkanDevice::InitVulkan(const SwapchainDesc& swapDesc) {
    VG_PROFILE_FUNCTION();
    VG_CORE_INFO("Initializing Vulkan Backend...");

    
    m_Instance.createInstance();

    
    m_PhysicalDevice.pickPhysicalDevice(*m_Instance.getInstance());

    
    VkSurfaceKHR c_surface;
    if (glfwCreateWindowSurface(*m_Instance.getInstance(), m_WindowHandle, nullptr, &c_surface) != VK_SUCCESS) {
        VG_CORE_ERROR("Failed to create window surface!");
        throw std::runtime_error("Failed to create window surface");
    }
    m_Surface = vk::raii::SurfaceKHR(*m_Instance.getInstance(), c_surface);

    // 4. Logical Device
    m_LogicalDevice.initialize(m_PhysicalDevice, *m_Surface, m_Instance);

    // 5. Swapchain
    m_SwapChain.createSwapChain(*m_LogicalDevice.getLogicalDevice(), 
                                m_PhysicalDevice, 
                                m_Surface, 
                                m_WindowHandle);

    // 6. Frame synchronization & Command Pools
    vk::raii::Device& device = *m_LogicalDevice.getLogicalDevice();
    
    // Find graphics queue family
    auto queueFamilies = m_PhysicalDevice.getPhysicalDevice().getQueueFamilyProperties();
    u32 graphicsQueueFamily = 0;
    for (u32 i = 0; i < queueFamilies.size(); i++) {
        if (queueFamilies[i].queueFlags & vk::QueueFlagBits::eGraphics) {
            graphicsQueueFamily = i;
            break;
        }
    }

    for (u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        m_Frames[i].Init(device, graphicsQueueFamily);
    }
    
    VG_CORE_INFO("Vulkan Backend Initialized Successfully");
}

void VulkanDevice::CleanupVulkan() {
    VG_PROFILE_FUNCTION();
    
    for (u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        m_Frames[i].Cleanup();
    }

    m_SwapChain.cleanupSwapChain();
    m_LogicalDevice.cleanup();
    m_Surface.clear();
    // m_Instance and m_PhysicalDevice clean themselves up via RAII/destructors
}

void VulkanDevice::WaitIdle() {
    m_LogicalDevice.getLogicalDevice()->waitIdle();
}

void VulkanDevice::BeginFrame() {
    VG_PROFILE_FUNCTION();

    vk::raii::Device& device = *m_LogicalDevice.getLogicalDevice();
    VulkanFrameContext& frame = m_Frames[m_CurrentFrame];

    // Wait for previous frame's GPU work to finish
    auto waitResult = device.waitForFences({*frame.inFlightFence}, VK_TRUE, UINT64_MAX);
    if (waitResult != vk::Result::eSuccess) {
        VG_CORE_ERROR("Failed to wait for in-flight fence");
    }

    // Flush deletion queue for this frame now that the GPU is done with it
    frame.deletionQueue.Flush();

    // Acquire next image from swapchain
    vk::Result acquireResult;
    try {
        auto [res, index] = m_SwapChain.GetSwapchain().acquireNextImage(UINT64_MAX, *frame.imageAvailableSemaphore, nullptr);
        acquireResult = res;
        m_ImageIndex = index;
    } catch (vk::OutOfDateKHRError&) {
        RecreateSwapchain();
        return; // Skip rendering this frame
    }

    if (acquireResult != vk::Result::eSuccess && acquireResult != vk::Result::eSuboptimalKHR) {
        throw std::runtime_error("Failed to acquire swap chain image!");
    }

    // Reset fence
    device.resetFences({*frame.inFlightFence});

    // Reset command buffer
    frame.commandBuffer.reset();

    // Begin recording
    vk::CommandBufferBeginInfo beginInfo{};
    frame.commandBuffer.begin(beginInfo);
}

void VulkanDevice::EndFrame() {
    VG_PROFILE_FUNCTION();

    FrameData& frame = m_Frames[m_CurrentFrame];
    frame.commandBuffer.end();

    // Submit command buffer
    vk::SubmitInfo submitInfo{};
    
    vk::Semaphore waitSemaphores[] = { *frame.imageAvailableSemaphore };
    vk::PipelineStageFlags waitStages[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    vk::CommandBuffer commandBuffers[] = { *frame.commandBuffer };
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = commandBuffers;

    vk::Semaphore signalSemaphores[] = { *frame.renderFinishedSemaphore };
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    auto queue = m_LogicalDevice.getGraphicsQueue();
    queue.submit({submitInfo}, *frame.inFlightFence);

    // Present
    vk::PresentInfoKHR presentInfo{};
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    vk::SwapchainKHR swapChains[] = { *m_SwapChain.GetSwapchain() };
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &m_ImageIndex;

    try {
        auto presentQueue = m_LogicalDevice.getPresentQueue();
        vk::Result presentResult = presentQueue.presentKHR(presentInfo);
        if (presentResult == vk::Result::eSuboptimalKHR || m_FramebufferResized) {
            m_FramebufferResized = false;
            RecreateSwapchain();
        }
    } catch (vk::OutOfDateKHRError&) {
        m_FramebufferResized = false;
        RecreateSwapchain();
    }

    m_CurrentFrame = (m_CurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void VulkanDevice::RecreateSwapchain() {
    VG_PROFILE_FUNCTION();
    
    int width = 0, height = 0;
    glfwGetFramebufferSize(m_WindowHandle, &width, &height);
    while (width == 0 || height == 0) {
        glfwGetFramebufferSize(m_WindowHandle, &width, &height);
        glfwWaitEvents();
    }

    WaitIdle();
    m_SwapChain.cleanupSwapChain();
    m_SwapChain.createSwapChain(*m_LogicalDevice.getLogicalDevice(), m_PhysicalDevice, m_Surface, m_WindowHandle);
}

// ─── Resource Management Stubs (Will be implemented in Phase 2) ───

BufferHandle VulkanDevice::CreateBuffer(const BufferDesc& desc) { return nullptr; }
TextureHandle VulkanDevice::CreateTexture(const TextureDesc& desc) { return nullptr; }
ShaderHandle VulkanDevice::CreateShader(const ShaderDesc& desc) { return nullptr; }
PipelineHandle VulkanDevice::CreateGraphicsPipeline(const GraphicsPipelineDesc& desc) { return nullptr; }

void VulkanDevice::DestroyBuffer(BufferHandle handle) {}
void VulkanDevice::DestroyTexture(TextureHandle handle) {}
void VulkanDevice::DestroyShader(ShaderHandle handle) {}
void VulkanDevice::DestroyPipeline(PipelineHandle handle) {}

void VulkanDevice::UpdateBuffer(BufferHandle handle, const void* data, u64 size, u64 offset) {}
void VulkanDevice::UpdateTexture(TextureHandle handle, const void* data, u64 size) {}
void* VulkanDevice::MapBuffer(BufferHandle handle) { return nullptr; }
void VulkanDevice::UnmapBuffer(BufferHandle handle) {}

void VulkanDevice::BeginRenderPass(const RenderPassDesc& desc) {}
void VulkanDevice::EndRenderPass() {}
void VulkanDevice::BindPipeline(PipelineHandle handle) {}
void VulkanDevice::BindVertexBuffer(BufferHandle handle, u64 offset) {}
void VulkanDevice::BindIndexBuffer(BufferHandle handle, u64 offset) {}
void VulkanDevice::SetViewport(float x, float y, float width, float height) {}
void VulkanDevice::SetScissor(i32 x, i32 y, u32 width, u32 height) {}
void VulkanDevice::PushConstants(ShaderStage stage, const void* data, u32 size, u32 offset) {}
void VulkanDevice::Draw(u32 vertexCount, u32 firstVertex) {}
void VulkanDevice::DrawIndexed(u32 indexCount, u32 firstIndex, i32 vertexOffset) {}

TextureHandle VulkanDevice::GetSwapchainImage() { return nullptr; }
TextureFormat VulkanDevice::GetSwapchainFormat() { return TextureFormat::BGRA8_SRGB; }
void VulkanDevice::ResizeSwapchain(u32 width, u32 height) { m_FramebufferResized = true; }
void VulkanDevice::BindTexture(u32 slot, TextureHandle handle) {}

} // namespace UHE::RHI
