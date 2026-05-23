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

       vk::raii::ImageView CreateImageView(vk::Image image, vk::Format format);
       
       void setPyhsicalDevice(vk::raii::PhysicalDevice &physicalDevice) {
         this->physicaldevice = &physicalDevice;
       }
       void LogicalDevice(vk::raii::Device &logicalDevice) {
         this->logicaldevice = &logicalDevice;
       }
       void setSwapChainSurfaceFormat(vk::SurfaceFormatKHR &swapChainSurfaceFormat) {
         this->swapChainSurfaceFormat = &swapChainSurfaceFormat;
       }

       void CreateImageViews();
       void CreateTextureImageView();
       void CreateTextureSampler();
       void
       CreateDepthImageView(vk::Format depthFormat = vk::Format::eD32Sfloat);

   private:
       vk::raii::PhysicalDevice* physicaldevice = nullptr;
       vk::raii::Device* logicaldevice = nullptr;
       vk::SurfaceFormatKHR* swapChainSurfaceFormat = nullptr;
       std::vector<vk::raii::ImageView> swapChainImageViews;
       std::vector<vk::Image> swapChainImages; 
       vk::raii::Sampler textureSampler;
   };
}
