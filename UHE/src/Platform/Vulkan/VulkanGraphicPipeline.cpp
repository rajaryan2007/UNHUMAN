#include "uhepch.h"
#include "VulkanGraphicPipeline.h"
#include "VulkanDescriptorManager.h"



namespace UHE::RHI {

void VulkanGraphicPipeline::Init() {}

void VulkanGraphicPipeline::Bind() {}

void VulkanGraphicPipeline::createGraphicsPipeline(
    VulkanLogicalDevice &Device, vk::Extent2D swapChainExtent,
    VulkanDescriptorManager &descriptorManager,
    vk::SurfaceFormatKHR swapChainSurfaceFormat, vk::Format depthFormat) 
{
  const auto &m_descriptorsetLayoutInfo = descriptorManager.GetLayoutHandle();
}

} // namespace UHE::RHI