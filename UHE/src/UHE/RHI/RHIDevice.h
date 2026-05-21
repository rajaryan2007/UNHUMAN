#pragma once
#include "RHITypes.h"
#include <memory>

namespace UHE::RHI {

class RHIDevice {
public:
    virtual ~RHIDevice() = default;
    static std::unique_ptr<RHIDevice> Create(Backend backend, const SwapchainDesc& swapDesc);

    
    virtual void Begin() = 0;
    virtual void End() = 0;

    // ─── Render Pass ───
    virtual void BeginRenderPass(const RenderPassDesc &desc) = 0;
    virtual void EndRenderPass() = 0;

    // ─── Pipeline & State Bindings ───
    virtual void BindPipeline(PipelineHandle handle) = 0;
    virtual void BindVertexBuffer(BufferHandle handle, u64 offset = 0) = 0;
    virtual void BindIndexBuffer(BufferHandle handle, u64 offset = 0) = 0;
    virtual void BindTexture(u32 slot, TextureHandle handle) = 0;

    // ─── Dynamic States ───
    virtual void SetViewport(float x, float y, float width, float height) = 0;
    virtual void SetScissor(i32 x, i32 y, u32 width, u32 height) = 0;

    // ─── Inline Data Paths ───
    virtual void PushConstants(ShaderStage stage, const void *data, u32 size,
                               u32 offset = 0) = 0;
    virtual void UpdateBuffer(BufferHandle handle, const void *data, u64 size,
                              u64 offset = 0) = 0;
    virtual void UpdateTexture(TextureHandle handle, const void *data,
                               u64 size) = 0;

    // ─── Action Commands ───
    virtual void Draw(u32 vertexCount, u32 firstVertex = 0) = 0;
    virtual void DrawIndexed(u32 indexCount, u32 firstIndex = 0,
                             i32 vertexOffset = 0) = 0;
};

} // namespace UHE::RHI
