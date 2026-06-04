#include "uhepch.h"
#include "VulkanDevice.h"
#include <GLFW/glfw3.h>
#include "UHE/Core/Log.h"

namespace UHE::RHI::VULKAN
{

void VulkanDevice::RecreateSwapchain()
{
    VG_PROFILE_FUNCTION();

    int width = 0, height = 0;
    glfwGetFramebufferSize(m_WindowHandle, &width, &height);
    while (width == 0 || height == 0)
    {
        glfwGetFramebufferSize(m_WindowHandle, &width, &height);
        glfwWaitEvents();
    }

    WaitIdle();
    m_SwapChain.cleanupSwapChain();
    m_SwapChain.createSwapChain(*m_LogicalDevice.getLogicalDevice(), m_PhysicalDevice, m_Surface, m_WindowHandle);
}

// ─── Resource Management Stubs  ───

BufferHandle VulkanDevice::CreateBuffer(const BufferDesc& desc)
{
    return nullptr;
}
TextureHandle VulkanDevice::CreateTexture(const TextureDesc& desc)
{
    return nullptr;
}
ShaderHandle VulkanDevice::CreateShader(const ShaderDesc& desc)
{
    return nullptr;
}
PipelineHandle VulkanDevice::CreateGraphicsPipeline(const GraphicsPipelineDesc& desc)
{
    return nullptr;
}

void VulkanDevice::BeginRenderPass(const RenderPassDesc& desc) {}
void VulkanDevice::EndRenderPass() {}
void VulkanDevice::BindPipeline(PipelineHandle handle) {}
void VulkanDevice::BindVertexBuffer(BufferHandle handle, u64 offset) {}
void VulkanDevice::BindIndexBuffer(BufferHandle handle, u64 offset) {}
void VulkanDevice::SetViewport(float x, float y, float width, float height) {}
void VulkanDevice::SetScissor(i32 x, i32 y, u32 width, u32 height) {}
void VulkanDevice::PushConstants(ShaderStage stage, const void* data, u32 size, u32 offset) {}
void VulkanDevice::DrawIndexed(u32 indexCount, u32 firstIndex, i32 vertexOffset) {}

void VulkanDevice::BindTexture(u32 slot, TextureHandle handle) {}

void VulkanDevice::UpdateBuffer(BufferHandle handle, const void* data, u64 size, u64 offset /*= 0*/) {}

void VulkanDevice::UpdateTexture(TextureHandle handle, const void* data, u64 size) {}

void VulkanDevice::ImmediateSubmit(std::function<void(vk::raii::CommandBuffer& cmd)>&& function)
{
    vk::CommandBufferAllocateInfo allocInfo{};
    allocInfo.commandPool = *m_UploadCommandPool;
    allocInfo.level = vk::CommandBufferLevel::ePrimary;
    allocInfo.commandBufferCount = 1;

    vk::raii::CommandBuffers cmdBuffers(m_logicalDevice, allocInfo);
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
    auto waitResult = m_logicalDevice.waitForFences({*m_UploadFence}, VK_TRUE, UINT64_MAX);
    VG_CORE_ASSERT(waitResult == vk::Result::eSuccess, "Failed to wait for upload fence!");

    m_logicalDevice.resetFences({*m_UploadFence});
    m_UploadCommandPool.reset();
}

} // namespace UHE::RHI::VULKAN
