#pragma once

namespace UHE::RHI::VULKAN
{
class VulkanRenderGraph
{
public:
    VulkanRenderGraph() = default;
    VulkanRenderGraph(const VulkanRenderGraph&) = delete;
    VulkanRenderGraph operator=(const VulkanRenderGraph&) = delete;
    ~VulkanRenderGraph() = default;

    void Init();
    void Execute();
    void Cleanup();

private:
};

} // namespace UHE::RHI::VULKAN
