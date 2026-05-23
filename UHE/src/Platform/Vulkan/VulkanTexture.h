#pragma once
#include "VulkanBackendState.h"
#include <vulkan/vulkan_raii.hpp>

namespace UHE::RHI {
class VulkanTexture {
public:
  VulkanTexture() = default;
  VulkanTexture(const VulkanTexture &) = delete;
  VulkanTexture &operator=(const VulkanTexture &) = delete;
  void Init(VulkanBackendState &backendState);
  void CreateTextureImage();
  void CreateTextureImageView();
  void CreateTextureSampler();

private:
  VulkanBackendState& m_backendState;

  vk::raii::Image textureImage;
  vk::raii::DeviceMemory textureImageMemory;
  vk::raii::ImageView textureImageView;
  vk::raii::Sampler textureSampler;
};
} // namespace UHE::RHI