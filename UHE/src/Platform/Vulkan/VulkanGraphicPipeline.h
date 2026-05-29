#pragma once
#include <vulkan/vulkan_raii.hpp>

namespace UHE::RHI {
class GraphicsPipelineDesc;
};

namespace UHE::RHI::VULKAN {
class VulkanLogicalDevice;
class VulkanDescriptorManager;


class VulkanGraphicPipeline {
public:
  VulkanGraphicPipeline();
  ~VulkanGraphicPipeline();

  void Init();
  void Bind();

private:
  void createGraphicsPipeline(VulkanLogicalDevice &Device,
                              VulkanDescriptorManager &descriptorManager,
                              const GraphicsPipelineDesc &desc);
  void createShaderModules();
  void cleanup();

  vk::raii::Device *m_Device;
  vk::raii::Pipeline *m_GraphicsPipeline;
  vk::raii::PipelineLayout *m_PipelineLayout;
  std::vector<vk::raii::ShaderModule> m_ShaderModules;
};
}
