#pragma once
#include "RHITypes.h"
#include <memory>

namespace UHE::RHI {

class RHICommandBuffer; // forward declaration

class RHIDevice {
public:
    virtual ~RHIDevice() = default;
    static std::unique_ptr<RHIDevice> Create(Backend backend, const SwapchainDesc& swapDesc);

    // ─── Frame Lifecycle ───────────────────────────────────────
    virtual void Begin() = 0;
    virtual void End() = 0;
    virtual void WaitIdle() = 0;
    virtual void ResetCommandBuffers() = 0;
    virtual uint32_t GetCurrentFrameIndex() const = 0;

    // ─── Resource Creation ─────────────────────────────────────
    virtual BufferHandle CreateBuffer(const BufferDesc &desc) = 0;
    virtual TextureHandle CreateTexture(const TextureDesc &desc) = 0;
    virtual ShaderHandle CreateShader(const ShaderDesc &desc) = 0;
    virtual PipelineHandle
    CreateGraphicsPipeline(const GraphicsPipelineDesc &desc) = 0;

    // ─── Data Transfer ─────────────────────────────────────────
    virtual void ReadPixel(TextureHandle handle, int x, int y, void* outData) = 0;

    // ─── Resource Destruction ──────────────────────────────────
    virtual void DestroyBuffer(BufferHandle handle) = 0;
    virtual void DestroyTexture(TextureHandle handle) = 0;
    virtual void DestroyShader(ShaderHandle handle) = 0;
    virtual void DestroyGraphicsPipeline(PipelineHandle handle) = 0;

    // ─── Command Buffer Access ─────────────────────────────────
    virtual RHICommandBuffer& GetCurrentCommandBuffer() = 0;
};

} // namespace UHE::RHI
