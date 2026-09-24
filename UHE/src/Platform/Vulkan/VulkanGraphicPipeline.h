#pragma once
#include <vulkan/vulkan_raii.hpp>
#include "Platform/Vulkan/VulkanContext.h"
#include "Platform/Vulkan/VulkanPipelineState.h"
#include "Platform/Vulkan/VulkanRenderPass.h"
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
class VulkanRenderPass;
class VulkanFramebuffer;

class VulkanGraphicPipeline : public VulkanPipelineState
{
public:
    VulkanGraphicPipeline();
    ~VulkanGraphicPipeline();
    VulkanGraphicPipeline(const VulkanGraphicPipeline&) = delete;
    VulkanGraphicPipeline operator=(const VulkanGraphicPipeline&) = delete;
    void Init();
    void Bind();

    void createGraphicsPipeline(VulkanLogicalDevice& Device, VulkanDescriptorManager& descriptorManager,
                                const VulkanContext& ctx, const GraphicsPipelineDesc& desc);

    [[nodiscard]] vk::Pipeline GetPipeline() const override { return *m_GraphicsPipeline; }
    [[nodiscard]] vk::PipelineLayout GetPipelineLayout() const override { return *m_PipelineLayout; }
    [[nodiscard]] vk::PipelineBindPoint GetBindPoint() const override { return vk::PipelineBindPoint::eGraphics; }
    [[nodiscard]] VulkanRenderPass& GetRenderPassHandle() { return m_FallbackRenderPass; }

private:
    vk::PipelineVertexInputStateCreateInfo CreateVertexInputState(const BufferLayout& layer);
    void createShaderModules();
    void cleanup();

    std::vector<vk::VertexInputBindingDescription> m_BindingDescription;
    std::vector<vk::VertexInputAttributeDescription> m_AttributeDescription;
    vk::raii::Pipeline m_GraphicsPipeline{nullptr};
    vk::raii::PipelineLayout m_PipelineLayout{nullptr};
    std::vector<vk::raii::ShaderModule> m_ShaderModules;
    VulkanRenderPass m_FallbackRenderPass; // Used when VK_KHR_dynamic_rendering is not available
};
} // namespace UHE::RHI::VULKAN
