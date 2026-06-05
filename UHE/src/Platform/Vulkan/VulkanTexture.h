#pragma once
#include <cstddef>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan_raii.hpp>
#include "UHE/RHI/RHITexture.h"

namespace UHE::RHI::VULKAN
{
class VulkanDevice;
class VulkanLogicalDevice;

class VulkanTexture : public RHITexture
{
public:
    VulkanTexture() = default;
    // VulkanTexture(const VulkanTexture&) = delete;
    // VulkanTexture& operator=(const VulkanTexture&) = delete;

    virtual const TextureDesc& GetDesc() const override;

    void Init(VulkanDevice& device);
    void CreateImage(VulkanLogicalDevice& device, uint32_t width, uint32_t height, vk::Format format,
                     vk::ImageUsageFlags usage, VmaMemoryUsage memUsage, vk::ImageTiling tiling, vk::raii::Image& image,
                     VmaAllocation& imageMemory);
    void CreateTexture(VulkanDevice& device, const void* pixelData, u32 width, u32 height, size_t dataSize);
    void ExecuteCopyCommand(const vk::raii::Device& device, VkBuffer srcBuffer, vk::Image dstImage, uint32_t width,
                            uint32_t height);

private:
    vk::raii::Image textureImage{nullptr};
    vk::raii::DeviceMemory textureImageMemory{nullptr};
    vk::raii::ImageView textureImageView{nullptr};
    vk::raii::Sampler textureSamplermi{nullptr};
    VmaAllocator m_allocator = nullptr;
};
} // namespace UHE::RHI::VULKAN
