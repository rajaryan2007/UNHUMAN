#include "uhepch.h"
#include "VulkanTexture.h"
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_raii.hpp>
#include "VulkanCommandPool.h"
#include "VulkanDevice.h"
#include "VulkanLogicalDevice.h"
#include "vulkan/vulkan.hpp"

namespace UHE::RHI::VULKAN
{

void VulkanTexture::Init(VulkanDevice& device) {}

void VulkanTexture::CreateImage(VulkanLogicalDevice& logDevice, uint32_t width, uint32_t height, vk::Format format,
                                vk::ImageUsageFlags usage, VmaMemoryUsage memUsage, vk::ImageTiling tiling,
                                vk::raii::Image& image, VmaAllocation& imageMemory)
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

    if (vmaCreateImage(logDevice.getAllocator(), &rawImageInfo, &allocInfo, &rawImage, &imageMemory, nullptr) !=
        VK_SUCCESS)
    {
        throw std::runtime_error("failed to create image via VMA!");
    }

    image = vk::raii::Image(logDevice.getLogicalDevice(), rawImage);
}

void VulkanTexture::CreateTexture(VulkanDevice& device, const void* pixelData, u32 height, u32 width, size_t dataSize)
{
    const auto& logicaldevice = &device.getLogicalDevClass().getLogicalDevice();
    m_allocator = device.getLogicalDevClass().getAllocator();

    VkBuffer stagingBuffer;
    VmaAllocation stagingAlloc;

    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = dataSize;
    bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

    VmaAllocationCreateInfo allocInfo{};
    allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
    allocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;

    vmaCreateBuffer(m_allocator, &bufferInfo, &allocInfo, &stagingBuffer, &stagingAlloc, nullptr);

    void* mapped;
    vmaMapMemory(m_allocator, stagingAlloc, &mapped);
    memcpy(mapped, pixelData, dataSize);
    vmaUnmapMemory(m_allocator, stagingAlloc);

    ExecuteCopyCommand(logicaldevice, stagingBuffer, *textureImage, width, height);
    vmaDestroyBuffer(m_allocator, stagingBuffer, stagingAlloc);
}

void VulkanTexture::ExecuteCopyCommand(const vk::raii::Device& device, VkBuffer srcBuffer, vk::Image dstImage,
                                       uint32_t width, uint32_t height)
{
}

} // namespace UHE::RHI::VULKAN
