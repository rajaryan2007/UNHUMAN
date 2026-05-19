#pragma once
#include <vulkan/vulkan_raii.hpp>

#include <vector>

namespace UHE::RHI {
   class VulkanDevice;
   class VulkanImageView {
   public: 
	   VulkanImageView() = default;
       VulkanImageView(const VulkanImageView &) = delete;
       VulkanImageView &operator=(const VulkanImageView &) = delete;

       vk::raii::ImageView CreateImageView(const vk::raii::Device &device,
                                           vk::Image image, vk::Format format);
       void CreateImageViews(VulkanDevice& device);
       void CreateTextureImageView(VulkanDevice& device);
       void CreateTextureSampler();
   private:
       std::vector<vk::raii::ImageView> swapChainImageViews;
       std::vector<vk::Image> swapChainImages; 
   };
}
