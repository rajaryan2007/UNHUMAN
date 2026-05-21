#pragma once 
#include <vk_mem_alloc.h>


namespace UHE::RHI {
class VulkanLogicalDevice;
class VulkanPhysicalDevice;
class VulkanGraphicsPipeline;
class VulkanSwapChain;
class VulkanTexture;
class VulkanBuffer;
class VulkanImageView;
class VulkanInstance;
class VulkanContext;

struct VulkanBackendState {
  VulkanLogicalDevice    &logicalDevice;
  VulkanPhysicalDevice   &physicalDevice;
  VulkanGraphicsPipeline &graphicsPipeline;
  VulkanSwapChain        &swapChain;
  VulkanTexture          &texture;
  VulkanBuffer           &buffer;
  VulkanImageView        &imageView;
  VulkanInstance         &instance;
  VulkanContext          &context;
  VmaAllocator           &allocator;
};
} // namespace UHE::RHI