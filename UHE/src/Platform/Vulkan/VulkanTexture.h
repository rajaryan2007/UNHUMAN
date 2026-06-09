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
    virtual ~VulkanTexture() override;

    virtual const TextureDesc& GetDesc() const override { return m_Desc; }
    virtual void* GetImGuiTextureID() override;


    void Init(VulkanDevice& device, const TextureDesc& desc);
    void CreateImage(VulkanLogicalDevice& device, uint32_t width, uint32_t height, vk::Format format,
                     vk::ImageUsageFlags usage, VmaMemoryUsage memUsage, vk::ImageTiling tiling, vk::raii::Image& image,
                     VmaAllocation& imageMemory);
    void CreateTexture(VulkanDevice& device, const void* pixelData, u32 width, u32 height, size_t dataSize);
    void ExecuteCopyCommand(VulkanDevice& device, VkBuffer srcBuffer, vk::Image dstImage, uint32_t width,
                            uint32_t height);
    void UpdateTexture(const void* data, size_t size);
    vk::raii::Image& GetImage() { return textureImage; }
    vk::raii::ImageView& GetImageView() { return textureImageView; }
    vk::raii::Sampler& GetSampler() { return textureSampler; }

private:
    vk::raii::Image textureImage{nullptr};
    VmaAllocation textureImageMemory = nullptr;
    vk::raii::ImageView textureImageView{nullptr};
    vk::raii::Sampler textureSampler{nullptr};
    VmaAllocator m_allocator = nullptr;
    VulkanDevice* m_Device = nullptr;
    u32 m_Width = 0;
    u32 m_Height = 0;
    TextureDesc m_Desc;
    VkDescriptorSet m_ImGuiDescriptorSet = VK_NULL_HANDLE;
};
} // namespace UHE::RHI::VULKAN
