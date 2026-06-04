#pragma once
#include <vulkan/vulkan_raii.hpp>
#include "UHE/RHI/RHITypes.h"
#include "vulkan/vulkan.hpp"

namespace UHE::RHI
{
class GraphicsPipelineDesc;
};

namespace UHE::RHI::VULKAN
{
class VulkanLogicalDevice;
class VulkanDescriptorManager;

class VulkanGraphicPipeline
{
public:
    VulkanGraphicPipeline();
    ~VulkanGraphicPipeline();

    void Init();
    void Bind();

    void createGraphicsPipeline(VulkanLogicalDevice& Device, VulkanDescriptorManager& descriptorManager,
                                const GraphicsPipelineDesc& desc);

    vk::Pipeline GetPipeline() const { return *m_GraphicsPipeline; }
    vk::PipelineLayout GetPipelineLayout() const { return *m_PipelineLayout; }

private:
    vk::PipelineVertexInputStateCreateInfo CreateVertexInputState(const BufferLayout& layer);
    void createShaderModules();
    void cleanup();

    std::vector<vk::VertexInputBindingDescription> m_BindingDescription;
    std::vector<vk::VertexInputAttributeDescription> m_AttributeDescription;
    vk::raii::Pipeline m_GraphicsPipeline;
    vk::raii::PipelineLayout m_PipelineLayout;
    std::vector<vk::raii::ShaderModule> m_ShaderModules;
};
} // namespace UHE::RHI::VULKAN
