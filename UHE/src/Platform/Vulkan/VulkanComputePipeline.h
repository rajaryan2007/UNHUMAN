#pragma once
#include <vulkan/vulkan_raii.hpp>
#include "Platform/Vulkan/VulkanPipelineState.h"
#include "UHE/RHI/RHITypes.h"

namespace UHE::RHI::VULKAN
{
class VulkanLogicalDevice;
class VulkanDescriptorManager;

class VulkanComputePipeline final : public VulkanPipelineState
{
public:
    VulkanComputePipeline() = default;
    ~VulkanComputePipeline() override = default;
    VulkanComputePipeline(const VulkanComputePipeline&) = delete;
    VulkanComputePipeline& operator=(const VulkanComputePipeline&) = delete;

    void CreateComputePipeline(VulkanLogicalDevice& Device, VulkanDescriptorManager& descriptorManager,
                               const ComputePipelineDesc& desc);
    void ShutDownComputePipeline();

    [[nodiscard]] vk::Pipeline GetPipeline() const override { return *m_ComputePipeline; }
    [[nodiscard]] vk::PipelineLayout GetPipelineLayout() const override { return *m_PipelineLayout; }
    [[nodiscard]] vk::PipelineBindPoint GetBindPoint() const override { return vk::PipelineBindPoint::eCompute; }

private:
    vk::raii::Pipeline m_ComputePipeline{nullptr};
    vk::raii::PipelineLayout m_PipelineLayout{nullptr};
};
} // namespace UHE::RHI::VULKAN
