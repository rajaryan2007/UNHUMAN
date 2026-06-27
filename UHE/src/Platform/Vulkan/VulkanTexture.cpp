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
    m_MipLevels = desc.mipLevels > 0 ? desc.mipLevels : 1;

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

    auto& logicaldevice = device.getLogicalDevClass().getLogicalDevice();
    m_allocator = device.getLogicalDevClass().getAllocator();

    CreateImage(device.getLogicalDevClass(), m_Width, m_Height, m_MipLevels, format, usage, VMA_MEMORY_USAGE_GPU_ONLY, vk::ImageTiling::eOptimal, textureImage, textureImageMemory);

    // Create Image View
    vk::ImageViewCreateInfo viewInfo{
        .flags = {},
        .image = textureImage,
        .viewType = vk::ImageViewType::e2D,
        .format = format,
        .components = {},
        .subresourceRange = {
            .aspectMask = aspect,
            .baseMipLevel = 0,
            .levelCount = m_MipLevels,
            .baseArrayLayer = 0,
            .layerCount = 1
        }
    };
    textureImageView = vk::raii::ImageView(logicaldevice, viewInfo);

    // Create Sampler (if Sampled)
    if (desc.usage & TextureUsage::Sampled) {
        vk::SamplerCreateInfo samplerInfo{
            .flags = {},
            .magFilter = vk::Filter::eLinear,
            .minFilter = vk::Filter::eLinear,
            .mipmapMode = vk::SamplerMipmapMode::eLinear,
            .addressModeU = vk::SamplerAddressMode::eRepeat,
            .addressModeV = vk::SamplerAddressMode::eRepeat,
            .addressModeW = vk::SamplerAddressMode::eRepeat,
            .mipLodBias = 0.0f,
            .anisotropyEnable = VK_FALSE,
            .maxAnisotropy = 1.0f,
            .compareEnable = VK_FALSE,
            .compareOp = vk::CompareOp::eAlways,
            .minLod = 0.0f,
            .maxLod = static_cast<float>(m_MipLevels),
            .borderColor = vk::BorderColor::eIntOpaqueBlack,
            .unnormalizedCoordinates = VK_FALSE
        };
        textureSampler = vk::raii::Sampler(logicaldevice, samplerInfo);
        if (!(aspect & vk::ImageAspectFlagBits::eDepth)) {
            m_TextureIndex = device.GetDescriptorManager()->BindTexture(device.getLogicalDevClass().getLogicalDevice(), *textureImageView, *textureSampler);
        }
    }
}

VulkanTexture::~VulkanTexture()
{
    if (m_Device) {
        m_Device->WaitIdle();
    }

    if (m_ImGuiDescriptorSet != VK_NULL_HANDLE && ImGui::GetCurrentContext() != nullptr)
    {
        ImGui_ImplVulkan_RemoveTexture(m_ImGuiDescriptorSet);
        m_ImGuiDescriptorSet = VK_NULL_HANDLE;
    }
    
    textureImageView.clear();
    textureSampler.clear();
    
    if (m_allocator && textureImage && textureImageMemory)
    {
        vmaDestroyImage(m_allocator, static_cast<VkImage>(textureImage), textureImageMemory);
        textureImage = nullptr;
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

void VulkanTexture::CreateImage(VulkanLogicalDevice& logDevice, uint32_t width, uint32_t height, uint32_t mipLevels, vk::Format format,
                                vk::ImageUsageFlags usage, VmaMemoryUsage memUsage, vk::ImageTiling tiling,
                                vk::Image& image, VmaAllocation& imageMemory)
{

    vk::ImageCreateInfo imageInfo{
        .flags = {},
        .imageType = vk::ImageType::e2D,
        .format = format,
        .extent = { width, height, 1 },
        .mipLevels = mipLevels,
        .arrayLayers = 1,
        .samples = vk::SampleCountFlagBits::e1,
        .tiling = tiling,
        .usage = usage,
        .sharingMode = vk::SharingMode::eExclusive,
        .queueFamilyIndexCount = 0,
        .pQueueFamilyIndices = nullptr,
        .initialLayout = vk::ImageLayout::eUndefined
    };

    VkImageCreateInfo rawImageInfo = static_cast<VkImageCreateInfo>(imageInfo);
    VmaAllocationCreateInfo allocInfo{
        .flags = 0,
        .usage = memUsage,
        .requiredFlags = 0,
        .preferredFlags = 0,
        .memoryTypeBits = 0,
        .pool = VK_NULL_HANDLE,
        .pUserData = nullptr,
        .priority = 0.0f
    };

    VkImage rawImage;

    if (vmaCreateImage(logDevice.getAllocator(), &rawImageInfo, &allocInfo, &rawImage, &imageMemory, nullptr) !=
        VK_SUCCESS)
    {
        throw std::runtime_error("failed to create image via VMA!");
    }
    
    image = rawImage;
}

void VulkanTexture::CreateTexture(VulkanDevice& device, const void* pixelData, u32 width, u32 height, size_t dataSize)
{
    m_Device = &device;
    m_Width = width;
    m_Height = height;

    auto& logicaldevice = device.getLogicalDevClass().getLogicalDevice();
    m_allocator = device.getLogicalDevClass().getAllocator();

    m_MipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(width, height)))) + 1;

    vk::ImageUsageFlags usage = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferSrc;
    VmaMemoryUsage memUsage = VMA_MEMORY_USAGE_GPU_ONLY;

    CreateImage(device.getLogicalDevClass(), width, height, m_MipLevels, vk::Format::eR8G8B8A8Srgb, usage, memUsage,
                vk::ImageTiling::eOptimal, textureImage, textureImageMemory);

    VkBuffer stagingBuffer;
    VmaAllocation stagingAlloc;

    VkBufferCreateInfo bufferInfo{
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .size = dataSize,
        .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0,
        .pQueueFamilyIndices = nullptr
    };

    VmaAllocationCreateInfo allocInfo{
        .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT,
        .usage = VMA_MEMORY_USAGE_AUTO,
        .requiredFlags = 0,
        .preferredFlags = 0,
        .memoryTypeBits = 0,
        .pool = VK_NULL_HANDLE,
        .pUserData = nullptr,
        .priority = 0.0f
    };

    vmaCreateBuffer(m_allocator, &bufferInfo, &allocInfo, &stagingBuffer, &stagingAlloc, nullptr);

    void* mapped;
    vmaMapMemory(m_allocator, stagingAlloc, &mapped);
    memcpy(mapped, pixelData, dataSize);
    vmaFlushAllocation(m_allocator, stagingAlloc, 0, VK_WHOLE_SIZE);
    vmaUnmapMemory(m_allocator, stagingAlloc);

    ExecuteCopyCommand(device, stagingBuffer, textureImage, width, height, m_MipLevels);
    vmaDestroyBuffer(m_allocator, stagingBuffer, stagingAlloc);

    // Create Image View
    vk::ImageViewCreateInfo viewInfo{
        .flags = {},
        .image = textureImage,
        .viewType = vk::ImageViewType::e2D,
        .format = vk::Format::eR8G8B8A8Srgb,
        .components = {},
        .subresourceRange = {
            .aspectMask = vk::ImageAspectFlagBits::eColor,
            .baseMipLevel = 0,
            .levelCount = m_MipLevels,
            .baseArrayLayer = 0,
            .layerCount = 1
        }
    };
    textureImageView = vk::raii::ImageView(logicaldevice, viewInfo);

    // Create Sampler
    vk::SamplerCreateInfo samplerInfo{
        .flags = {},
        .magFilter = vk::Filter::eLinear,
        .minFilter = vk::Filter::eLinear,
        .mipmapMode = vk::SamplerMipmapMode::eLinear,
        .addressModeU = vk::SamplerAddressMode::eRepeat,
        .addressModeV = vk::SamplerAddressMode::eRepeat,
        .addressModeW = vk::SamplerAddressMode::eRepeat,
        .mipLodBias = 0.0f,
        .anisotropyEnable = VK_FALSE,
        .maxAnisotropy = 1.0f,
        .compareEnable = VK_FALSE,
        .compareOp = vk::CompareOp::eAlways,
        .minLod = 0.0f,
        .maxLod = static_cast<float>(m_MipLevels),
        .borderColor = vk::BorderColor::eIntOpaqueBlack,
        .unnormalizedCoordinates = VK_FALSE
    };
    textureSampler = vk::raii::Sampler(logicaldevice, samplerInfo);
    
    m_TextureIndex = device.GetDescriptorManager()->BindTexture(logicaldevice, *textureImageView, *textureSampler);
}

void VulkanTexture::ExecuteCopyCommand(VulkanDevice& device, VkBuffer srcBuffer, vk::Image dstImage,
                                       uint32_t width, uint32_t height, uint32_t mipLevels)
{
    device.ImmediateSubmit([&](vk::raii::CommandBuffer& cmd) {
        // Transition to TRANSFER_DST
        vk::ImageMemoryBarrier barrier{
            .srcAccessMask = {},
            .dstAccessMask = vk::AccessFlagBits::eTransferWrite,
            .oldLayout = vk::ImageLayout::eUndefined,
            .newLayout = vk::ImageLayout::eTransferDstOptimal,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image = dstImage,
            .subresourceRange = {
                .aspectMask = vk::ImageAspectFlagBits::eColor,
                .baseMipLevel = 0,
                .levelCount = mipLevels,
                .baseArrayLayer = 0,
                .layerCount = 1
            }
        };

        cmd.pipelineBarrier(vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eTransfer, {}, nullptr, nullptr, barrier);

        // Copy
        vk::BufferImageCopy region{
            .bufferOffset = 0,
            .bufferRowLength = 0,
            .bufferImageHeight = 0,
            .imageSubresource = {
                .aspectMask = vk::ImageAspectFlagBits::eColor,
                .mipLevel = 0,
                .baseArrayLayer = 0,
                .layerCount = 1
            },
            .imageOffset = {0, 0, 0},
            .imageExtent = vk::Extent3D{width, height, 1}
        };

        cmd.copyBufferToImage(srcBuffer, dstImage, vk::ImageLayout::eTransferDstOptimal, region);

        if (mipLevels == 1) {
            // Transition to SHADER_READ_ONLY if no mipmaps to generate
            barrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
            barrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
            barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
            barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

            cmd.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader, {}, nullptr, nullptr, barrier);
        }
    });

    if (mipLevels > 1) {
        GenerateMipmaps(device, dstImage, vk::Format::eR8G8B8A8Srgb, width, height, mipLevels);
    }
}

void VulkanTexture::GenerateMipmaps(VulkanDevice& device, vk::Image image, vk::Format imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels)
{
    device.ImmediateSubmit([&](vk::raii::CommandBuffer& cmd) {
        vk::ImageMemoryBarrier barrier{
            .srcAccessMask = {},
            .dstAccessMask = {},
            .oldLayout = vk::ImageLayout::eUndefined,
            .newLayout = vk::ImageLayout::eUndefined,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image = image,
            .subresourceRange = {
                .aspectMask = vk::ImageAspectFlagBits::eColor,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1
            }
        };

        int32_t mipWidth = texWidth;
        int32_t mipHeight = texHeight;

        for (uint32_t i = 1; i < mipLevels; i++) {
            barrier.subresourceRange.baseMipLevel = i - 1;
            barrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
            barrier.newLayout = vk::ImageLayout::eTransferSrcOptimal;
            barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
            barrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;

            cmd.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eTransfer, {}, nullptr, nullptr, barrier);

            vk::ImageBlit blit{
                .srcSubresource = {
                    .aspectMask = vk::ImageAspectFlagBits::eColor,
                    .mipLevel = i - 1,
                    .baseArrayLayer = 0,
                    .layerCount = 1
                },
                .srcOffsets = {{ vk::Offset3D{0, 0, 0}, vk::Offset3D{mipWidth, mipHeight, 1} }},
                .dstSubresource = {
                    .aspectMask = vk::ImageAspectFlagBits::eColor,
                    .mipLevel = i,
                    .baseArrayLayer = 0,
                    .layerCount = 1
                },
                .dstOffsets = {{ vk::Offset3D{0, 0, 0}, vk::Offset3D{ mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 } }}
            };

            cmd.blitImage(image, vk::ImageLayout::eTransferSrcOptimal, image, vk::ImageLayout::eTransferDstOptimal, blit, vk::Filter::eNearest);

            barrier.oldLayout = vk::ImageLayout::eTransferSrcOptimal;
            barrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
            barrier.srcAccessMask = vk::AccessFlagBits::eTransferRead;
            barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

            cmd.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader, {}, nullptr, nullptr, barrier);

            if (mipWidth > 1) mipWidth /= 2;
            if (mipHeight > 1) mipHeight /= 2;
        }

        barrier.subresourceRange.baseMipLevel = mipLevels - 1;
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

    VkBufferCreateInfo bufferInfo{
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .size = size,
        .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0,
        .pQueueFamilyIndices = nullptr
    };

    VmaAllocationCreateInfo allocInfo{
        .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT,
        .usage = VMA_MEMORY_USAGE_AUTO,
        .requiredFlags = 0,
        .preferredFlags = 0,
        .memoryTypeBits = 0,
        .pool = VK_NULL_HANDLE,
        .pUserData = nullptr,
        .priority = 0.0f
    };

    vmaCreateBuffer(m_allocator, &bufferInfo, &allocInfo, &stagingBuffer, &stagingAlloc, nullptr);

    void* mapped;
    vmaMapMemory(m_allocator, stagingAlloc, &mapped);
    memcpy(mapped, data, size);
    vmaFlushAllocation(m_allocator, stagingAlloc, 0, VK_WHOLE_SIZE);
    vmaUnmapMemory(m_allocator, stagingAlloc);

    ExecuteCopyCommand(*m_Device, stagingBuffer, textureImage, m_Width, m_Height, m_MipLevels);
    vmaDestroyBuffer(m_allocator, stagingBuffer, stagingAlloc);
}

} // namespace UHE::RHI::VULKAN
