#pragma once
#include <vulkan/vulkan_raii.hpp>

namespace UHE::RHI::VULKAN
{
class VulkanPipelineState
{
public:
    VulkanPipelineState() = default;
    virtual ~VulkanPipelineState() = default;
    VulkanPipelineState(const VulkanPipelineState&) = delete;
    VulkanPipelineState& operator=(const VulkanPipelineState&) = delete;

    [[nodiscard]] virtual vk::Pipeline GetPipeline() const = 0;
    [[nodiscard]] virtual vk::PipelineLayout GetPipelineLayout() const = 0;
    [[nodiscard]] virtual vk::PipelineBindPoint GetBindPoint() const = 0;

    void Init();
    void Shutdown();

private:
};
} // namespace UHE::RHI::VULKAN
