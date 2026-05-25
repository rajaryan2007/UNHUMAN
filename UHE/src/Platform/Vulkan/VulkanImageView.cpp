#include "uhepch.h"
#include "VulkanImageView.h"
#include <vk_mem_alloc.h>

namespace UHE::RHI {
   vk::raii::ImageView VulkanImageView::CreateImageView(vk::Image image,
       vk::Format format) 
   {
	   if (!image)
	   {
         throw std::runtime_error("Invalid image handle");
	   }

	   vk::ImageViewCreateInfo imageViewCreateinfo{};
           imageViewCreateinfo.viewType = vk::ImageViewType::e2D;
           imageViewCreateinfo.image    = image;
           imageViewCreateinfo.format = format;
           imageViewCreateinfo.subresourceRange.aspectMask =
               vk::ImageAspectFlagBits::eColor;
           imageViewCreateinfo.subresourceRange.baseMipLevel = 0;
           imageViewCreateinfo.subresourceRange.levelCount = 1;
           imageViewCreateinfo.subresourceRange.baseArrayLayer = 0;
           imageViewCreateinfo.subresourceRange.layerCount = 1;
           
           return vk::raii::ImageView(*logicaldevice, imageViewCreateinfo);
   }

   void VulkanImageView::CreateImageViews() 
   {   
       swapChainImageViews.clear();

       vk::ImageViewCreateInfo m_imageViewCreateInfo{};
        m_imageViewCreateInfo.viewType = vk::ImageViewType::e2D;
        m_imageViewCreateInfo.format = swapChainSurfaceFormat->format; 
        m_imageViewCreateInfo.subresourceRange.aspectMask =
           vk::ImageAspectFlagBits::eColor;
        m_imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
        m_imageViewCreateInfo.subresourceRange.levelCount = 1;
        m_imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
        
        for (const auto& image : swapChainImages)
        {
          m_imageViewCreateInfo.image = image;
          swapChainImageViews.emplace_back(
              vk::raii::ImageView(*logicaldevice, m_imageViewCreateInfo)
          );
        }

   }

   void VulkanImageView::CreateTextureImageView() 
   {
      
   }

   void VulkanImageView::CreateTextureSampler() 
   {
     vk::PhysicalDeviceProperties properties = physicaldevice->getProperties();

     vk::SamplerCreateInfo samplerInfo{};
     samplerInfo.magFilter = vk::Filter::eLinear;
     samplerInfo.minFilter = vk::Filter::eLinear;
     samplerInfo.addressModeU = vk::SamplerAddressMode::eRepeat;
     samplerInfo.addressModeV = vk::SamplerAddressMode::eRepeat;
     samplerInfo.addressModeW = vk::SamplerAddressMode::eRepeat;
     samplerInfo.anisotropyEnable = VK_TRUE;
     samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
     samplerInfo.borderColor = vk::BorderColor::eIntOpaqueBlack;
     samplerInfo.unnormalizedCoordinates = VK_FALSE;
     samplerInfo.compareEnable = VK_FALSE;
     samplerInfo.compareOp = vk::CompareOp::eAlways;
     samplerInfo.mipmapMode = vk::SamplerMipmapMode::eLinear;
     textureSampler = vk::raii::Sampler(*logicaldevice, samplerInfo);
   }

   void VulkanImageView::CreateDepthImageView(vk::Format depthFormat,
                                              vk::Extent2D swapChainExtent)
   {
     vk::ImageCreateInfo imageinfo{};
     imageinfo.imageType = vk::ImageType::e2D;
     imageinfo.extent.width = swapChainExtent.width;
     imageinfo.extent.height = swapChainExtent.height;
     imageinfo.extent.depth = 1;
     imageinfo.mipLevels = 1;
     imageinfo.arrayLayers = 1;
     imageinfo.format = depthFormat;
     imageinfo.tiling = vk::ImageTiling::eOptimal;
     imageinfo.initialLayout = vk::ImageLayout::eUndefined;
     imageinfo.usage = vk::ImageUsageFlagBits::eDepthStencilAttachment;
     imageinfo.samples = vk::SampleCountFlagBits::e1;
     imageinfo.sharingMode = vk::SharingMode::eExclusive;

     VkImageCreateInfo rawImageInfo =
         static_cast<VkImageCreateInfo>(imageinfo);
     VmaAllocationCreateInfo allocInfo{};
     allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

    
     if (vmaCreateImage(m_allocator, &rawImageInfo, &allocInfo, &rawImage,
                        &depthImageAllocation,
                        NULL) != VK_SUCCESS) {
         throw std::runtime_error("Failed to create depth image");
     }

     depthImage = vk::raii::Image(*logicaldevice, rawImage); 
     rawHandle = depthImage.release();

     vk::ImageViewCreateInfo DepthViewInfo{};
     DepthViewInfo.viewType = vk::ImageViewType::e2D;
     DepthViewInfo.format = depthFormat;
     DepthViewInfo.image = *depthImage;
     DepthViewInfo.subresourceRange.aspectMask =
         vk::ImageAspectFlagBits::eDepth;
     DepthViewInfo.subresourceRange.baseMipLevel = 0;
     DepthViewInfo.subresourceRange.levelCount = 1;
     DepthViewInfo.subresourceRange.baseArrayLayer = 0;
     DepthViewInfo.subresourceRange.layerCount = 1;

     depthImageView = vk::raii::ImageView(*logicaldevice, DepthViewInfo);

   }

   VulkanImageView::~VulkanImageView()
   {
     if (depthImage != nullptr) {
       vmaDestroyImage(m_allocator, rawImage,
                       depthImageAllocation);
       depthImage.release();
       depthImageAllocation = nullptr;
     }
     depthImageView = nullptr;
   }
   //void VulkanImageView::CreateImageViews() {}   
   } // namespace UHE