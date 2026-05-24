#include "uhepch.h"
#include "VulkanTexture.h"
#include "VulkanDevice.h"
#include "VulkanLogicalDevice.h"

namespace UHE::RHI {
	
  
void VulkanTexture::Init(VulkanDevice &device)
{

}

void VulkanTexture::CreateImage(VulkanLogicalDevice &logDevice, uint32_t width,
                                uint32_t height, vk::Format format,
                                vk::ImageUsageFlags usage,
                                VmaMemoryUsage memUsage, vk::ImageTiling tiling,
                                vk::raii::Image &image,
                                VmaAllocation &imageMemory) 
{
  
  vk::ImageCreateInfo imageInfo{};
  imageInfo.imageType = vk::ImageType::e2D;
  imageInfo.extent.width = width;
  imageInfo.extent.height = height;
  imageInfo.extent.depth = 1;
  imageInfo.mipLevels = 1;
  imageInfo.arrayLayers = 1;
  imageInfo.format = format;
  imageInfo.tiling = tiling;
  imageInfo.initialLayout = vk::ImageLayout::eUndefined;
  imageInfo.usage = usage;
  imageInfo.samples = vk::SampleCountFlagBits::e1;
  imageInfo.sharingMode = vk::SharingMode::eExclusive;

  VkImageCreateInfo rawImageInfo = static_cast<VkImageCreateInfo>(imageInfo);
  VmaAllocationCreateInfo allocInfo{};
  allocInfo.usage = memUsage;

  VkImage rawImage;
  VkImage rawImage;
  if (vmaCreateImage(&LogDevice.GetAllocator(), &rawImageInfo, &allocInfo,
                     &rawImage, &imageMemory, nullptr) != VK_SUCCESS) {
    throw std::runtime_error("failed to create image via VMA!");
  }


}

void VulkanTexture::CreateTexture(VulkanDevice &device)
{

}

} // namespace UHE::RHI