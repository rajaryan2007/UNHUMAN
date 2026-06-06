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

    // ─── Resource Creation ─────────────────────────────────────
    virtual BufferHandle CreateBuffer(const BufferDesc &desc) = 0;
    virtual TextureHandle CreateTexture(const TextureDesc &desc) = 0;
    virtual ShaderHandle CreateShader(const ShaderDesc &desc) = 0;
    virtual PipelineHandle
    CreateGraphicsPipeline(const GraphicsPipelineDesc &desc) = 0;

    // ─── Command Buffer Access ─────────────────────────────────
    virtual RHICommandBuffer& GetCurrentCommandBuffer() = 0;
};

} // namespace UHE::RHI
