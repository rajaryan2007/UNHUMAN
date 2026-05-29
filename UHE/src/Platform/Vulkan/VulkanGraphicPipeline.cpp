#include "uhepch.h"
#include "VulkanGraphicPipeline.h"
#include "VulkanDescriptorManager.h"
#include "VulkanShader.h"
#include "UHE/RHI/RHITypes.h"



namespace UHE::RHI::VULKAN {

void VulkanGraphicPipeline::Init() {}

void VulkanGraphicPipeline::Bind() {}

void VulkanGraphicPipeline::createGraphicsPipeline(
    VulkanLogicalDevice &Device, 
    VulkanDescriptorManager &descriptorManager,
    const GraphicsPipelineDesc& desc) 
{
  const auto& m_descriptorsetLayoutInfo = descriptorManager.GetLayoutHandle();
  auto vertModule = static_cast<VulkanShader *>(desc.vertexShader)->GetModule();
  auto fragModule = static_cast<VulkanShader *>(desc.fragmentShader)->GetModule();
  vk::PipelineShaderStageCreateInfo shaderStages[] = 
  {   
      {{},vk::ShaderStageFlagBits::eVertex,vertModule},
      {{},vk::ShaderStageFlagBits::eFragment,fragModule}
  };

  vk::PipelineVertexInputStateCreateInfo vertexInputInfo =
      CreateVertexInputState(desc.vertexLayout);

  vk::PipelineRasterizationStateCreateInfo rasterizer{};
  rasterizer.polygonMode =
      desc.Wireframe ? vk::PolygonMode::eLine : vk::PolygonMode::eFill;
  rasterizer.cullMode = desc.CullMode; 
  rasterizer.frontFace = vk::FrontFace::eCounterClockwise;

  auto globalLayout = descriptorManager.GetLayoutHandle();
  vk::PipelineLayoutCreateInfo pipelineLayoutInfo{};
  pipelineLayoutInfo.setLayoutCount = 1;
  pipelineLayoutInfo.pSetLayouts = &globalLayout;

  vk::PushConstantRange pushConstantRange{};
  pushConstantRange.stageFlags = vk::ShaderStageFlagBits::eAllGraphics;
  pushConstantRange.size = sizeof(uint32_t) * 4; // Example: 4 integers
  pipelineLayoutInfo.pushConstantRangeCount = 1;
  pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

  m_PipelineLayout = vk::raii::PipelineLayout(device.GetRawDevice(), pipelineLayoutInfo);

} // namespace UHE::RHI