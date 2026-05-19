#pragma once

namespace UHE {
class VulkanRenderGraph 
{
  VulkanRenderGraph() = default;
  VulkanRenderGraph(const VulkanRenderGraph &) = delete; 
  VulkanRenderGraph &operator=(const VulkanRenderGraph &) = delete;

  void Init();
  void Execute();
  void Cleanup();
};
}




