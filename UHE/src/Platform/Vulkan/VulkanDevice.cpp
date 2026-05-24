#include "uhepch.h"
#include "VulkanDevice.h"
#include "UHE/Core/Log.h"
#include <GLFW/glfw3.h>

namespace UHE::RHI {



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

// ─── Resource Management Stubs  ───

BufferHandle VulkanDevice::CreateBuffer(const BufferDesc& desc) { return nullptr; }
TextureHandle VulkanDevice::CreateTexture(const TextureDesc& desc) { return nullptr; }
ShaderHandle VulkanDevice::CreateShader(const ShaderDesc& desc) { return nullptr; }
PipelineHandle VulkanDevice::CreateGraphicsPipeline(const GraphicsPipelineDesc& desc) { return nullptr; }



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


void VulkanDevice::BindTexture(u32 slot, TextureHandle handle) {}

void VulkanDevice::UpdateBuffer(BufferHandle handle, const void *data, u64 size,
                                u64 offset /*= 0*/) {}

void VulkanDevice::UpdateTexture(TextureHandle handle, const void *data,
                                 u64 size) {}

} // namespace UHE::RHI
