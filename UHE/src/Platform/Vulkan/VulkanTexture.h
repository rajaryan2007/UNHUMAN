#pragma once
#include "UHE/RHI/RHITexture.h"
#include <vulkan/vulkan_raii.hpp>


namespace UHE::RHI {
class VulkanDevice;
class VulkanLogicalDevice;

class VulkanTexture : public RHITexture {
public:
  VulkanTexture() = default;
  VulkanTexture(const VulkanTexture &) = delete;
  VulkanTexture &operator=(const VulkanTexture &) = delete;

  virtual const TextureDesc &GetDesc() const override;

  void Init(VulkanDevice &device);
  void CreateImage(VulkanLogicalDevice &device, uint32_t width,
                                  uint32_t height, vk::Format format,
                                  vk::ImageUsageFlags usage,
                                  VmaMemoryUsage memUsage,
                                  vk::ImageTiling tiling,
                                  vk::raii::Image &image,
                                  VmaAllocation &imageMemory);
  void CreateTexture(VulkanDevice &device);
  

private:
  

  vk::raii::Image textureImage;
  vk::raii::DeviceMemory textureImageMemory;
  vk::raii::ImageView textureImageView;
  vk::raii::Sampler textureSampler;
};
} // namespace UHE::RHI