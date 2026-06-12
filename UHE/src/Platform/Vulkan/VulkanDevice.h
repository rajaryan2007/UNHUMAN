#pragma once
#include <vk_mem_alloc.h>
#include <vulkan/vulkan_raii.hpp>
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
    VulkanDevice(const SwapchainDesc& swapDesc);
    ~VulkanDevice() override;

    // ─── Frame Lifecycle ────────────────────────────────────────
    void Begin() override;
    void End() override;

    // ─── Resource Creation ──────────────────────────────────────
    BufferHandle CreateBuffer(const BufferDesc& desc) override;
    TextureHandle CreateTexture(const TextureDesc& desc) override;
    ShaderHandle CreateShader(const ShaderDesc& desc) override;
    PipelineHandle CreateGraphicsPipeline(const GraphicsPipelineDesc& desc) override;

    void ReadPixel(TextureHandle handle, int x, int y, void* outData) override;

    void DestroyBuffer(BufferHandle handle) override;
    void DestroyTexture(TextureHandle handle) override;
    void DestroyShader(ShaderHandle handle) override;
    void DestroyGraphicsPipeline(PipelineHandle handle) override;

    // ─── Command Buffer Access ──────────────────────────────────
    RHICommandBuffer& GetCurrentCommandBuffer() override;

    // ─── Window Management ────────────────────────────────────────
    [[nodiscard]] GLFWwindow* GetWindowHandle() const { return m_WindowHandle; }

    void GetLogicalDeviceInfo(u32& vendorID, u32& deviceID) const
    {
        return m_PhysicalDevice.GetLogicalDeviceInfo(vendorID, deviceID);
    };

    [[nodiscard]] vk::raii::Queue& GetGraphicsQueue() { return m_LogicalDevice.getGraphicsQueue(); }
    [[nodiscard]] VulkanLogicalDevice& getLogicalDevClass() { return m_LogicalDevice; }
    [[nodiscard]] VulkanInstance& getInstanceClass() { return m_Instance; }
    [[nodiscard]] VulkanPhysicalDevice& getPhysicalDevClass() { return m_PhysicalDevice; }
    [[nodiscard]] VulkanSwapChain& getSwapChainClass() { return m_SwapChain; }
    [[nodiscard]] const u32& ImageIndex() { return m_ImageIndex; }
    void ImmediateSubmit(std::function<void(vk::raii::CommandBuffer& cmd)>&& function);
    void WaitIdle() override;

private:
    void InitVulkan(const SwapchainDesc& swapDesc);
    void CleanupVulkan();
    void RecreateSwapchain();

    // ─── Immediate Submission (Uploads) ───────────────────────────

private:
    // Core Vulkan abstractions
    VulkanInstance m_Instance;
    VulkanPhysicalDevice m_PhysicalDevice;
    VulkanLogicalDevice m_LogicalDevice;
    VulkanSwapChain m_SwapChain;
    VmaAllocator m_Allocator = nullptr;
    VulkanDescriptorManager m_DescriptorManager;

    GLFWwindow* m_WindowHandle = nullptr;
    u32 m_WindowWidth;
    u32 m_WindowHeight;

    // Frame-in-flight sync
    static constexpr u32 MAX_FRAMES_IN_FLIGHT = 2;
    std::array<VulkanFrameContext, MAX_FRAMES_IN_FLIGHT> m_Frames;
    std::vector<vk::raii::Semaphore> m_RenderFinishedSemaphores;
    u32 m_CurrentFrame = 0;
    u32 m_ImageIndex = 0; // Current swapchain image index
    bool m_FramebufferResized = false;

    // Immediate submit context
    vk::raii::Fence m_UploadFence = nullptr;
    vk::raii::CommandPool m_UploadCommandPool = nullptr;
    vk::raii::CommandBuffer m_UploadCommandBuffer = nullptr;

    class VulkanGraphicPipeline* m_CurrentPipeline = nullptr;

    enum class VendorID
    {
        VENDOR_ID_AMD = 0x1002,
        VENDOR_ID_NVIDIA = 0x10de,
        VENDOR_ID_INTEL = 0x8086,
        VENDOR_ID_ARM = 0x13b5,
        VENDOR_ID_QCOM = 0x5143
    };
};

} // namespace UHE::RHI::VULKAN
