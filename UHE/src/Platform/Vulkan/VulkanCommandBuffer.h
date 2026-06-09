#pragma once
#include <vulkan/vulkan_raii.hpp>
#include "Platform/Vulkan/VulkanSwapChain.h"
#include "UHE/RHI/RHICommadBuffer.h"
// #include "VulkanCommandPool.h"

namespace UHE::RHI::VULKAN
{
class VulkanDevice;
class VulkanCommandPool;
class VulkanDescriptorManager;

class VulkanCommandBuffer : public RHICommandBuffer
{
public:
    VulkanCommandBuffer() = default;
    ~VulkanCommandBuffer() override = default;

    VulkanCommandBuffer(const VulkanCommandBuffer&) = delete;
    VulkanCommandBuffer& operator=(const VulkanCommandBuffer&) = delete;

    VulkanCommandBuffer(VulkanCommandBuffer&&) = default;
    VulkanCommandBuffer& operator=(VulkanCommandBuffer&&) = default;

    // ─── Internal Vulkan-specific methods ───────────────────────
    void Allocate(const vk::raii::Device& device, VulkanCommandPool& pool, bool isPrimary = true);
    void Free();
    void BeginCommandBuffer(vk::CommandBufferUsageFlags flags = {});
    void Reset(vk::CommandBufferUsageFlags flags = {});
    void EndCommandBuffer();

    inline const vk::raii::CommandBuffer& GetHandle() const { return m_CommandBuffer; }
    inline vk::raii::CommandBuffer& GetHandle() { return m_CommandBuffer; }

    // ─── RHICommandBuffer overrides ─────────────────────────────
    virtual void Begin() override;
    virtual void End() override;

    // ─── Render Pass ───
    virtual void BeginRenderPass(const RenderPassDesc& desc) override;
    virtual void EndRenderPass() override;

    // ─── Pipeline & State Bindings ───
    virtual void BindPipeline(PipelineHandle handle) override;
    virtual void BindVertexBuffer(BufferHandle handle, u64 offset = 0) override;
    virtual void BindIndexBuffer(BufferHandle handle, u64 offset = 0) override;
    virtual void BindTexture(u32 slot, TextureHandle handle) override;

    // ─── Dynamic States ───
    virtual void SetViewport(float x, float y, float width, float height) override;
    virtual void SetScissor(i32 x, i32 y, u32 width, u32 height) override;

    // ─── Inline Data Paths ───
    virtual void PushConstants(ShaderStage stage, const void* data, u32 size, u32 offset = 0) override;
    virtual void UpdateBuffer(BufferHandle handle, const void* data, u64 size, u64 offset = 0) override;
    virtual void UpdateTexture(TextureHandle handle, const void* data, u64 size) override;

    // ─── Action Commands ───
    virtual void Draw(u32 vertexCount, u32 firstVertex = 0) override;
    virtual void DrawIndexed(u32 indexCount, u32 firstIndex = 0, i32 vertexOffset = 0) override;

    void SetContext(vk::raii::Device* device, VulkanDescriptorManager* descriptorManager)
    {
        m_LogDevice = device;
        m_DescriptorManager = descriptorManager;
    }

private:
    vk::raii::Device* m_LogDevice = nullptr;
    VulkanDescriptorManager* m_DescriptorManager = nullptr;
    vk::PipelineLayout m_CurrentPipelineLayout = nullptr;
    RenderPassDesc m_CurrentRenderPassDesc;
    vk::raii::CommandBuffer m_CommandBuffer{nullptr};
};
} // namespace UHE::RHI::VULKAN
