#pragma once
#include <vk_mem_alloc.h>
#include "Platform/Vulkan/VulkanDescriptorManager.h"
#include "Platform/Vulkan/VulkanFrameContext.h"
#include "Platform/Vulkan/VulkanInstance.h"
#include "Platform/Vulkan/VulkanLogicalDevice.h"
#include "Platform/Vulkan/VulkanPhysicalDevice.h"
#include "Platform/Vulkan/VulkanSwapChain.h"
#include "UHE/RHI/RHIDevice.h"

namespace UHE::RHI::VULKAN
{

class VulkanDevice final : public RHIDevice
{
public:
    VulkanDevice(GLFWwindow* windowHandle, const SwapchainDesc& swapDesc);
    ~VulkanDevice() override;

    // ─── Resource Creation ──────────────────────────────────────
    void Begin() override;
    void End() override;
    BufferHandle CreateBuffer(const BufferDesc& desc) override;
    TextureHandle CreateTexture(const TextureDesc& desc) override;
    ShaderHandle CreateShader(const ShaderDesc& desc) override;
    PipelineHandle CreateGraphicsPipeline(const GraphicsPipelineDesc& desc) override;

    // ─── Window Management ────────────────────────────────────────
    GLFWwindow* GetWindowHandle() const { return m_WindowHandle; }

    // ─── Data Upload ────────────────────────────────────────────
    void UpdateBuffer(BufferHandle handle, const void* data, u64 size, u64 offset = 0) override;
    void UpdateTexture(TextureHandle handle, const void* data, u64 size) override;

    // ─── Command Recording ──────────────────────────────────────
    void BeginRenderPass(const RenderPassDesc& desc) override;
    void EndRenderPass() override;

    void BindPipeline(PipelineHandle handle) override;
    void BindVertexBuffer(BufferHandle handle, u64 offset = 0) override;
    void BindIndexBuffer(BufferHandle handle, u64 offset = 0) override;

    void SetViewport(float x, float y, float width, float height) override;
    void SetScissor(i32 x, i32 y, u32 width, u32 height) override;
    void PushConstants(ShaderStage stage, const void* data, u32 size, u32 offset = 0) override;

    void Draw(u32 vertexCount, u32 firstVertex = 0) override {}

    void DrawIndexed(u32 indexCount, u32 firstIndex = 0, i32 vertexOffset = 0) override;

    void GetLogicalDeviceInfo(u32& vendorID, u32& deviceID) const
    {
        return m_PhysicalDevice.GetLogicalDeviceInfo(vendorID, deviceID);
    };

    vk::raii::Device& GetVulkanDevice() { return m_LogDevice; }

    void BindTexture(u32 slot, TextureHandle handle) override;

    VulkanLogicalDevice& getLogicalDevClass() { return m_LogicalDevice; }
    VulkanInstance& getInstanceClass() { return m_Instance; }
    VulkanPhysicalDevice& getPhysicalDevClass() { return m_PhysicalDevice; }
    VulkanSwapChain& getSwapChainClass() { return m_SwapChain; }

private:
    void InitVulkan(const SwapchainDesc& swapDesc);
    void CleanupVulkan();
    void RecreateSwapchain();

    // ─── Immediate Submission (Uploads) ───────────────────────────
    void ImmediateSubmit(std::function<void(vk::raii::CommandBuffer& cmd)>&& function);

private:
    // Core Vulkan abstractions
    vk::raii::Device& m_LogDevice;
    vk::raii::Queue& m_graphicsQueue;
    vk::raii::SurfaceKHR& surface;

    VulkanInstance m_Instance;
    VulkanPhysicalDevice m_PhysicalDevice;
    VulkanLogicalDevice m_LogicalDevice;
    vk::raii::SurfaceKHR m_Surface = nullptr;
    VulkanSwapChain m_SwapChain;
    VmaAllocator m_Allocator = nullptr;
    VulkanDescriptorManager m_DescriptorManager;

    GLFWwindow* m_WindowHandle = nullptr;
    u32 m_WindowWidth;
    u32 m_WindowHeight;

    // Frame-in-flight sync
    static constexpr u32 MAX_FRAMES_IN_FLIGHT = 2;
    std::array<VulkanFrameContext, MAX_FRAMES_IN_FLIGHT> m_Frames;
    u32 m_CurrentFrame = 0;
    u32 m_ImageIndex = 0; // Current swapchain image index
    bool m_FramebufferResized = false;

    // Immediate submit context
    vk::raii::Fence m_UploadFence = nullptr;
    vk::raii::CommandPool m_UploadCommandPool = nullptr;
    vk::raii::CommandBuffer m_UploadCommandBuffer = nullptr;

    class VulkanGraphicPipeline* m_CurrentPipeline = nullptr;

    enum VendorID
    {
        VENDOR_ID_AMD = 0x1002,
        VENDOR_ID_NVIDIA = 0x10de,
        VENDOR_ID_INTEL = 0x8086,
        VENDOR_ID_ARM = 0x13b5,
        VENDOR_ID_QCOM = 0x5143
    };
};

} // namespace UHE::RHI::VULKAN
