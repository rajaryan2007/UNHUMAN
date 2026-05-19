#pragma once
#include "RHITypes.h"
#include <memory>

namespace UHE::RHI {

class RHIDevice {
public:
    virtual ~RHIDevice() = default;

    /// Create a device for the given backend. Returns nullptr on failure.
    static std::unique_ptr<RHIDevice> Create(Backend backend, const SwapchainDesc& swapDesc);

    

    virtual BufferHandle   CreateBuffer(const BufferDesc& desc) = 0;
    virtual TextureHandle  CreateTexture(const TextureDesc& desc) = 0;
    virtual ShaderHandle   CreateShader(const ShaderDesc& desc) = 0;
    virtual PipelineHandle CreateGraphicsPipeline(const GraphicsPipelineDesc& desc) = 0;

    virtual void DestroyBuffer(BufferHandle handle) = 0;
    virtual void DestroyTexture(TextureHandle handle) = 0;
    virtual void DestroyShader(ShaderHandle handle) = 0;
    virtual void DestroyPipeline(PipelineHandle handle) = 0;

  


    virtual void UpdateBuffer(BufferHandle handle, const void* data, u64 size, u64 offset = 0) = 0;

    
    virtual void UpdateTexture(TextureHandle handle, const void* data, u64 size) = 0;

   
    virtual void* MapBuffer(BufferHandle handle) = 0;
    virtual void  UnmapBuffer(BufferHandle handle) = 0;

    // ─── Frame Lifecycle ────────────────────────────────────────

    /// Acquire next swapchain image, wait on previous frame fence, reset command buffer.
    virtual void BeginFrame() = 0;

    /// Submit command buffer, present swapchain image.
    virtual void EndFrame() = 0;

    /// Block until all GPU work is done. Call before shutdown.
    virtual void WaitIdle() = 0;

    // Command Recording 

    virtual void BeginRenderPass(const RenderPassDesc& desc) = 0;
    virtual void EndRenderPass() = 0;

    virtual void BindPipeline(PipelineHandle handle) = 0;
    virtual void BindVertexBuffer(BufferHandle handle, u64 offset = 0) = 0;
    virtual void BindIndexBuffer(BufferHandle handle, u64 offset = 0) = 0;

    virtual void SetViewport(float x, float y, float width, float height) = 0;
    virtual void SetScissor(i32 x, i32 y, u32 width, u32 height) = 0;

    /// Push constants  fast per-draw data path (128 bytes guaranteed).
    virtual void PushConstants(ShaderStage stage, const void* data, u32 size, u32 offset = 0) = 0;

    virtual void Draw(u32 vertexCount, u32 firstVertex = 0) = 0;
    virtual void DrawIndexed(u32 indexCount, u32 firstIndex = 0, i32 vertexOffset = 0) = 0;

    // ─── Swapchain ──────────────────────────────────────────────

    virtual TextureHandle GetSwapchainImage() = 0;
    virtual TextureFormat GetSwapchainFormat() = 0;
    virtual void ResizeSwapchain(u32 width, u32 height) = 0;

    // ─── Descriptors ────────────────────────────────────────────

    
    virtual void BindTexture(u32 slot, TextureHandle handle) = 0;

    // ─── Info ───────────────────────────────────────────────────

    virtual Backend GetBackend() const = 0;
    virtual u32 GetCurrentFrameIndex() const = 0;
};

} // namespace UHE::RHI
