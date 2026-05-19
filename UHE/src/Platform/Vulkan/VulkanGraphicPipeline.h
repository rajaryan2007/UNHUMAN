#pragma once
#include <vulkan/vulkan_raii.hpp>

namespace UHE {
class VulkanGraphicPipeline {
public:
  VulkanGraphicPipeline();
  ~VulkanGraphicPipeline();

  void Init();
  void Bind();

private:
  void createGraphicsPipeline();
  void createShaderModules();
  void cleanup();

  vk::raii::Device *m_Device;
  vk::raii::Pipeline *m_GraphicsPipeline;
  vk::raii::PipelineLayout *m_PipelineLayout;
  std::vector<vk::raii::ShaderModule> m_ShaderModules;
};
}
