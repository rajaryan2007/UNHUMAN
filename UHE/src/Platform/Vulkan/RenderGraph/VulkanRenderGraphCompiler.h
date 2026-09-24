#pragma once

namespace UHE::RHI::VULKAN
{
class VulkanRenderGraphCompiler
{
public:
    VulkanRenderGraphCompiler() = default;
    VulkanRenderGraphCompiler(const VulkanRenderGraphCompiler&) = delete;
    VulkanRenderGraphCompiler operator=(const VulkanRenderGraphCompiler&) = delete;
    ~VulkanRenderGraphCompiler() = default;
    
    void InitCompiler();
    void ShutdownCompiler();
    void CompileResourcesBarriers();
    void CompileRenderPass();
    void CompileBufferBarriers();
    void CompileImageBarriers();
    void CompileQueueBarriers();
    void CompileAsyncComputepass();
    void CompileAsyncTransferpass();
    void CompileSemaphoreSynchronization();
    void findPreviousScope();
    void optimizeBarriers();


private:
};

} // namespace UHE::RHI::VULKAN
