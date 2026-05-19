#pragma once
#include "UHE/RHI/RHIDevice.h"
#include "Platform/Vulkan/VulkanFrameContext.h"

// Vulkan wrappers
#include "Platform/Vulkan/VulkanInstance.h"
#include "Platform/Vulkan/VulkanPhysicalDevice.h"
#include "Platform/Vulkan/VulkanLogicalDevice.h"
#include "Platform/Vulkan/VulkanSwapChain.h"

#include <vk_mem_alloc.h>
#include <array>
#include <memory>

namespace UHE::RHI {

class VulkanDevice final : public RHIDevice {
public:
    VulkanDevice(const SwapchainDesc& swapDesc);
    ~VulkanDevice() override;

    // ─── Resource Creation ──────────────────────────────────────
    BufferHandle   CreateBuffer(const BufferDesc& desc) override;
    TextureHandle  CreateTexture(const TextureDesc& desc) override;
    ShaderHandle   CreateShader(const ShaderDesc& desc) override;
    PipelineHandle CreateGraphicsPipeline(const GraphicsPipelineDesc& desc) override;

    // ─── Resource Destruction ───────────────────────────────────
    void DestroyBuffer(BufferHandle handle) override;
    void DestroyTexture(TextureHandle handle) override;
    void DestroyShader(ShaderHandle handle) override;
    void DestroyPipeline(PipelineHandle handle) override;

    // ─── Data Upload ────────────────────────────────────────────
    void UpdateBuffer(BufferHandle handle, const void* data, u64 size, u64 offset = 0) override;
    void UpdateTexture(TextureHandle handle, const void* data, u64 size) override;
    void* MapBuffer(BufferHandle handle) override;
    void  UnmapBuffer(BufferHandle handle) override;

    // ─── Frame Lifecycle ────────────────────────────────────────
    void BeginFrame() override;
    void EndFrame() override;
    void WaitIdle() override;

    // ─── Command Recording ──────────────────────────────────────
    void BeginRenderPass(const RenderPassDesc& desc) override;
    void EndRenderPass() override;

    void BindPipeline(PipelineHandle handle) override;
    void BindVertexBuffer(BufferHandle handle, u64 offset = 0) override;
    void BindIndexBuffer(BufferHandle handle, u64 offset = 0) override;

    void SetViewport(float x, float y, float width, float height) override;
    void SetScissor(i32 x, i32 y, u32 width, u32 height) override;
    void PushConstants(ShaderStage stage, const void* data, u32 size, u32 offset = 0) override;

    void Draw(u32 vertexCount, u32 firstVertex = 0) override;
    void DrawIndexed(u32 indexCount, u32 firstIndex = 0, i32 vertexOffset = 0) override;

    // ─── Swapchain ──────────────────────────────────────────────
    TextureHandle GetSwapchainImage() override;
    TextureFormat GetSwapchainFormat() override;
    void ResizeSwapchain(u32 width, u32 height) override;

    // ─── Descriptors ────────────────────────────────────────────
    void BindTexture(u32 slot, TextureHandle handle) override;

    // ─── Info ───────────────────────────────────────────────────
    Backend GetBackend() const override { return Backend::Vulkan; }
    u32 GetCurrentFrameIndex() const override { return m_CurrentFrame; }

    // ─── Internal Vulkan Access (for backend only) ──────────────
    inline vk::raii::Device& GetDevice() { return *m_LogicalDevice.getLogicalDevice(); }
    inline VmaAllocator GetAllocator() { return m_LogicalDevice.getAllocator(); }
    inline DeletionQueue& GetCurrentDeletionQueue() { return m_Frames[m_CurrentFrame].deletionQueue; }

private:
    void InitVulkan(const SwapchainDesc& swapDesc);
    void CleanupVulkan();
    void RecreateSwapchain();

private:
    // Core Vulkan abstractions
    instance_vk          m_Instance;
    PhysicalDevice       m_PhysicalDevice;
    VkLogicalDevice      m_LogicalDevice;
    vk::raii::SurfaceKHR m_Surface = nullptr;
    VulkanSwapChain      m_SwapChain;

    GLFWwindow*          m_WindowHandle = nullptr;
    u32                  m_WindowWidth;
    u32                  m_WindowHeight;

    // Frame-in-flight sync
    static constexpr u32 MAX_FRAMES_IN_FLIGHT = 2;
    std::array<VulkanFrameContext, MAX_FRAMES_IN_FLIGHT> m_Frames;
    u32 m_CurrentFrame = 0;
    u32 m_ImageIndex = 0; // Current swapchain image index
    bool m_FramebufferResized = false;

    enum VendorID {
      VENDOR_ID_AMD = 0x1002,
      VENDOR_ID_NVIDIA = 0x10de,
      VENDOR_ID_INTEL = 0x8086,
      VENDOR_ID_ARM = 0x13b5,
      VENDOR_ID_QCOM = 0x5143
    };
};

} // namespace UHE::RHI
