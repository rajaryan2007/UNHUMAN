#pragma once

namespace UHE::RHI::VULKAN
{
class VulkanPipelineState
{
public:
    VulkanPipelineState() = default;
    ~VulkanPipelineState() = default;
    VulkanPipelineState(VulkanPipelineState&) = delete;
    VulkanPipelineState operator=(VulkanPipelineState&) = delete;

    void Init();
    void Shutdown();

private:
};
} // namespace UHE::RHI::VULKAN
