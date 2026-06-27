#include "uhepch.h"
#include "VulkanImageView.h"
#include <vk_mem_alloc.h>

namespace UHE::RHI::VULKAN
{
vk::raii::ImageView VulkanImageView::CreateImageView(vk::Image image, vk::Format format)
{
    if (!image)
    {
        throw std::runtime_error("Invalid image handle");
    }

    vk::ImageViewCreateInfo imageViewCreateinfo{
        .flags = {},
        .image = image,
        .viewType = vk::ImageViewType::e2D,
        .format = format,
        .components = {},
        .subresourceRange = {
            .aspectMask = vk::ImageAspectFlagBits::eColor,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1
        }
    };

    return vk::raii::ImageView(*logicaldevice, imageViewCreateinfo);
}

void VulkanImageView::CreateImageViews()
{
    swapChainImageViews.clear();

    vk::ImageViewCreateInfo m_imageViewCreateInfo{
        .flags = {},
        .image = {}, // Will be set in loop
        .viewType = vk::ImageViewType::e2D,
        .format = swapChainSurfaceFormat->format,
        .components = {},
        .subresourceRange = {
            .aspectMask = vk::ImageAspectFlagBits::eColor,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1
        }
    };

    for (const auto& image : swapChainImages)
    {
        m_imageViewCreateInfo.image = image;
        swapChainImageViews.emplace_back(vk::raii::ImageView(*logicaldevice, m_imageViewCreateInfo));
    }
}

void VulkanImageView::CreateTextureImageView() {}

void VulkanImageView::CreateTextureSampler()
{
    vk::PhysicalDeviceProperties properties = physicaldevice->getProperties();

    vk::SamplerCreateInfo samplerInfo{
        .flags = {},
        .magFilter = vk::Filter::eLinear,
        .minFilter = vk::Filter::eLinear,
        .mipmapMode = vk::SamplerMipmapMode::eLinear,
        .addressModeU = vk::SamplerAddressMode::eRepeat,
        .addressModeV = vk::SamplerAddressMode::eRepeat,
        .addressModeW = vk::SamplerAddressMode::eRepeat,
        .mipLodBias = 0.0f,
        .anisotropyEnable = VK_TRUE,
        .maxAnisotropy = properties.limits.maxSamplerAnisotropy,
        .compareEnable = VK_FALSE,
        .compareOp = vk::CompareOp::eAlways,
        .minLod = 0.0f,
        .maxLod = 0.0f,
        .borderColor = vk::BorderColor::eIntOpaqueBlack,
        .unnormalizedCoordinates = VK_FALSE
    };
    textureSampler = vk::raii::Sampler(*logicaldevice, samplerInfo);
}

void VulkanImageView::CreateDepthImageView(vk::Extent2D swapChainExtent, vk::Format depthFormat)
{
    vk::ImageCreateInfo imageinfo{
        .flags = {},
        .imageType = vk::ImageType::e2D,
        .format = depthFormat,
        .extent = {
            .width = swapChainExtent.width,
            .height = swapChainExtent.height,
            .depth = 1
        },
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = vk::SampleCountFlagBits::e1,
        .tiling = vk::ImageTiling::eOptimal,
        .usage = vk::ImageUsageFlagBits::eDepthStencilAttachment,
        .sharingMode = vk::SharingMode::eExclusive,
        .queueFamilyIndexCount = 0,
        .pQueueFamilyIndices = nullptr,
        .initialLayout = vk::ImageLayout::eUndefined
    };

    VkImageCreateInfo rawImageInfo = static_cast<VkImageCreateInfo>(imageinfo);
    VmaAllocationCreateInfo allocInfo{
        .flags = 0,
        .usage = VMA_MEMORY_USAGE_GPU_ONLY,
        .requiredFlags = 0,
        .preferredFlags = 0,
        .memoryTypeBits = 0,
        .pool = VK_NULL_HANDLE,
        .pUserData = nullptr,
        .priority = 0.0f
    };

    if (vmaCreateImage(m_allocator, &rawImageInfo, &allocInfo, &rawImage, &depthImageAllocation, NULL) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create depth image");
    }

    depthImage = vk::raii::Image(*logicaldevice, rawImage);
    rawHandle = depthImage.release();

    vk::ImageViewCreateInfo DepthViewInfo{
        .flags = {},
        .image = *depthImage,
        .viewType = vk::ImageViewType::e2D,
        .format = depthFormat,
        .components = {},
        .subresourceRange = {
            .aspectMask = vk::ImageAspectFlagBits::eDepth,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1
        }
    };

    depthImageView = vk::raii::ImageView(*logicaldevice, DepthViewInfo);
}

VulkanImageView::~VulkanImageView()
{
    if (depthImage != nullptr)
    {
        vmaDestroyImage(m_allocator, rawImage, depthImageAllocation);
        depthImage.release();
        depthImageAllocation = nullptr;
    }
    depthImageView = nullptr;
}
// void VulkanImageView::CreateImageViews() {}
} // namespace UHE::RHI::VULKAN
