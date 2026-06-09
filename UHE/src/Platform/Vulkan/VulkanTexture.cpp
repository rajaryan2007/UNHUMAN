#include "uhepch.h"
#include "VulkanTexture.h"
#include <backends/imgui_impl_vulkan.h>
#include <imgui.h>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_raii.hpp>
#include "VulkanCommandPool.h"
#include "VulkanDevice.h"
#include "VulkanLogicalDevice.h"
#include "vulkan/vulkan.hpp"

namespace UHE::RHI::VULKAN
{

void VulkanTexture::Init(VulkanDevice& device, const TextureDesc& desc) {
    m_Device = &device;
    m_Desc = desc;
    m_Width = desc.width;
    m_Height = desc.height;

    vk::Format format = vk::Format::eR8G8B8A8Srgb;
    switch(desc.format) {
        case TextureFormat::RGBA8_UNORM: format = vk::Format::eR8G8B8A8Unorm; break;
        case TextureFormat::RGBA8_SRGB: format = vk::Format::eR8G8B8A8Srgb; break;
        case TextureFormat::BGRA8_UNORM: format = vk::Format::eB8G8R8A8Unorm; break;
        case TextureFormat::BGRA8_SRGB: format = vk::Format::eB8G8R8A8Srgb; break;
        case TextureFormat::R8_UNORM: format = vk::Format::eR8Unorm; break;
        case TextureFormat::RG8_UNORM: format = vk::Format::eR8G8Unorm; break;
        case TextureFormat::RGBA16F: format = vk::Format::eR16G16B16A16Sfloat; break;
        case TextureFormat::RGBA32F: format = vk::Format::eR32G32B32A32Sfloat; break;
        case TextureFormat::R32_SINT: format = vk::Format::eR32Sint; break;
        case TextureFormat::D24_UNORM_S8: format = vk::Format::eD32SfloatS8Uint; break;
        case TextureFormat::D32_FLOAT: format = vk::Format::eD32Sfloat; break;
        default: break;
    }

    vk::ImageUsageFlags usage{};
    if (desc.usage & TextureUsage::Sampled) usage |= vk::ImageUsageFlagBits::eSampled;
    if (desc.usage & TextureUsage::ColorAttach) usage |= vk::ImageUsageFlagBits::eColorAttachment;
    if (desc.usage & TextureUsage::DepthAttach) usage |= vk::ImageUsageFlagBits::eDepthStencilAttachment;
    if (desc.usage & TextureUsage::Storage) usage |= vk::ImageUsageFlagBits::eStorage;
    if (desc.usage & TextureUsage::TransferSrc) usage |= vk::ImageUsageFlagBits::eTransferSrc;
    if (desc.usage & TextureUsage::TransferDst) usage |= vk::ImageUsageFlagBits::eTransferDst;

    vk::ImageAspectFlags aspect = vk::ImageAspectFlagBits::eColor;
    if (desc.usage & TextureUsage::DepthAttach) {
        aspect = vk::ImageAspectFlagBits::eDepth;
        if (format == vk::Format::eD32SfloatS8Uint || format == vk::Format::eD24UnormS8Uint)
            aspect |= vk::ImageAspectFlagBits::eStencil;
    }

    const auto& logicaldevice = device.getLogicalDevClass().getLogicalDevice();
    m_allocator = device.getLogicalDevClass().getAllocator();

    CreateImage(device.getLogicalDevClass(), m_Width, m_Height, format, usage, VMA_MEMORY_USAGE_GPU_ONLY, vk::ImageTiling::eOptimal, textureImage, textureImageMemory);

    // Create Image View
    vk::ImageViewCreateInfo viewInfo{};
    viewInfo.image = *textureImage;
    viewInfo.viewType = vk::ImageViewType::e2D;
    viewInfo.format = format;
    viewInfo.subresourceRange.aspectMask = aspect;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;
    textureImageView = vk::raii::ImageView(logicaldevice, viewInfo);

    // Create Sampler (if Sampled)
    if (desc.usage & TextureUsage::Sampled) {
        vk::SamplerCreateInfo samplerInfo{};
        samplerInfo.magFilter = vk::Filter::eLinear;
        samplerInfo.minFilter = vk::Filter::eLinear;
        samplerInfo.addressModeU = vk::SamplerAddressMode::eRepeat;
        samplerInfo.addressModeV = vk::SamplerAddressMode::eRepeat;
        samplerInfo.addressModeW = vk::SamplerAddressMode::eRepeat;
        samplerInfo.anisotropyEnable = VK_FALSE;
        samplerInfo.borderColor = vk::BorderColor::eIntOpaqueBlack;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;
        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = vk::CompareOp::eAlways;
        samplerInfo.mipmapMode = vk::SamplerMipmapMode::eLinear;
        textureSampler = vk::raii::Sampler(logicaldevice, samplerInfo);
    }
}

VulkanTexture::~VulkanTexture()
{
    if (m_allocator && textureImageMemory)
    {
        vmaFreeMemory(m_allocator, textureImageMemory);
        textureImageMemory = nullptr;
    }
}

void* VulkanTexture::GetImGuiTextureID()
{
    if (m_ImGuiDescriptorSet == VK_NULL_HANDLE)
    {
        VkSampler sampler = *textureSampler;
        VkImageView imageView = *textureImageView;
        m_ImGuiDescriptorSet = ImGui_ImplVulkan_AddTexture(sampler, imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    }
    return (void*)m_ImGuiDescriptorSet;
}

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

void VulkanTexture::CreateTexture(VulkanDevice& device, const void* pixelData, u32 width, u32 height, size_t dataSize)
{
    m_Device = &device;
    m_Width = width;
    m_Height = height;

    const auto& logicaldevice = device.getLogicalDevClass().getLogicalDevice();
    m_allocator = device.getLogicalDevClass().getAllocator();

    vk::ImageUsageFlags usage = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled;
    VmaMemoryUsage memUsage = VMA_MEMORY_USAGE_GPU_ONLY;

    CreateImage(device.getLogicalDevClass(), width, height, vk::Format::eR8G8B8A8Srgb, usage, memUsage,
                vk::ImageTiling::eOptimal, textureImage, textureImageMemory);

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

    ExecuteCopyCommand(device, stagingBuffer, *textureImage, width, height);
    vmaDestroyBuffer(m_allocator, stagingBuffer, stagingAlloc);

    // Create Image View
    vk::ImageViewCreateInfo viewInfo{};
    viewInfo.image = *textureImage;
    viewInfo.viewType = vk::ImageViewType::e2D;
    viewInfo.format = vk::Format::eR8G8B8A8Srgb;
    viewInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;
    textureImageView = vk::raii::ImageView(logicaldevice, viewInfo);

    // Create Sampler
    vk::SamplerCreateInfo samplerInfo{};
    samplerInfo.magFilter = vk::Filter::eLinear;
    samplerInfo.minFilter = vk::Filter::eLinear;
    samplerInfo.addressModeU = vk::SamplerAddressMode::eRepeat;
    samplerInfo.addressModeV = vk::SamplerAddressMode::eRepeat;
    samplerInfo.addressModeW = vk::SamplerAddressMode::eRepeat;
    samplerInfo.anisotropyEnable = VK_FALSE;
    samplerInfo.borderColor = vk::BorderColor::eIntOpaqueBlack;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = vk::CompareOp::eAlways;
    samplerInfo.mipmapMode = vk::SamplerMipmapMode::eLinear;
    textureSampler = vk::raii::Sampler(logicaldevice, samplerInfo);
}

void VulkanTexture::ExecuteCopyCommand(VulkanDevice& device, VkBuffer srcBuffer, vk::Image dstImage,
                                       uint32_t width, uint32_t height)
{
    device.ImmediateSubmit([&](vk::raii::CommandBuffer& cmd) {
        // Transition to TRANSFER_DST
        vk::ImageMemoryBarrier barrier{};
        barrier.oldLayout = vk::ImageLayout::eUndefined;
        barrier.newLayout = vk::ImageLayout::eTransferDstOptimal;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = dstImage;
        barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;
        barrier.srcAccessMask = {};
        barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;

        cmd.pipelineBarrier(vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eTransfer, {}, nullptr, nullptr, barrier);

        // Copy
        vk::BufferImageCopy region{};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;
        region.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;
        region.imageExtent = vk::Extent3D{width, height, 1};

        cmd.copyBufferToImage(srcBuffer, dstImage, vk::ImageLayout::eTransferDstOptimal, region);

        // Transition to SHADER_READ_ONLY
        barrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
        barrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
        barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
        barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

        cmd.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader, {}, nullptr, nullptr, barrier);
    });
}

void VulkanTexture::UpdateTexture(const void* data, size_t size)
{
    if (!m_Device) return;

    const auto& logicaldevice = m_Device->getLogicalDevClass().getLogicalDevice();
    m_allocator = m_Device->getLogicalDevClass().getAllocator();

    VkBuffer stagingBuffer;
    VmaAllocation stagingAlloc;

    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

    VmaAllocationCreateInfo allocInfo{};
    allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
    allocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;

    vmaCreateBuffer(m_allocator, &bufferInfo, &allocInfo, &stagingBuffer, &stagingAlloc, nullptr);

    void* mapped;
    vmaMapMemory(m_allocator, stagingAlloc, &mapped);
    memcpy(mapped, data, size);
    vmaUnmapMemory(m_allocator, stagingAlloc);

    ExecuteCopyCommand(*m_Device, stagingBuffer, *textureImage, m_Width, m_Height);
    vmaDestroyBuffer(m_allocator, stagingBuffer, stagingAlloc);
}

} // namespace UHE::RHI::VULKAN
