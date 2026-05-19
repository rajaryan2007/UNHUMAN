#include "uhepch.h"
#include "VulkanImageView.h"


namespace UHE {
   vk::raii::ImageView VulkanImageView::CreateImageView(const vk::raii::Device &device,
	   vk::Image image,vk::Format format) 
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
           
           return vk::raii::ImageView(device, imageViewCreateinfo);
   }

   void VulkanImageView::CreateImageViews() 
   {   
       swapChainImageViews.clear();

       const auto &device = ctx.device;

       vk::ImageViewCreateInfo m_imageViewCreateInfo{};
        m_imageViewCreateInfo.viewType = vk::ImageViewType::e2D;
        m_imageViewCreateInfo.format = swapChainSurfaceFormat.format; 
        m_imageViewCreateInfo.subresourceRange.aspectMask =
           vk::ImageAspectFlagBits::eColor;
        m_imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
        m_imageViewCreateInfo.subresourceRange.levelCount = 1;
        m_imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
        
        for (const auto& image : swapChainImages)
        {
          m_imageViewCreateInfo.image = image;
          swapChainImageViews.emplace_back(
              vk::raii::ImageView(device, m_imageViewCreateInfo)
          );
        }

   }

   void VulkanImageView::CreateTextureImageView(vulkanctx& ctx) 
   {
     const auto &physicaldevice = ctx.physicalDevice;

     vk::PhysicalDeviceProperties properties = physicaldevice.getProperties();

   }

   void VulkanImageView::CreateTextureSampler() 
   {

   }
} // namespace UHE