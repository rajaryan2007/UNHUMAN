#include "uhepch.h"
#include "VulkanDescriptorManager.h"
#include "VulkanDevice.h"
#include "vulkan/vulkan.hpp"

namespace UHE::RHI::VULKAN
{

void VulkanDescriptorManager::init(VulkanDevice& device)
{
    const auto& logicaldevice = device.getLogicalDevClass().getLogicalDevice();

    const uint32_t MAX_BINDLESS_RESOURCES = 10000;

    std::array<vk::DescriptorPoolSize, 2> poolSizes = {
        vk::DescriptorPoolSize{vk::DescriptorType::eStorageBuffer, MAX_BINDLESS_RESOURCES},
        vk::DescriptorPoolSize{vk::DescriptorType::eCombinedImageSampler, MAX_BINDLESS_RESOURCES}};

    vk::DescriptorPoolCreateInfo poolInfo{
        .flags = vk::DescriptorPoolCreateFlagBits::eUpdateAfterBind | vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet, // alllow updating descriptors after they've been bound
        .maxSets = 2,
        // poolInfo.maxSets = MAX_BINDLESS_RESOURCES * 2;          // max number of descriptor sets
        .poolSizeCount = static_cast<uint32_t>(poolSizes.size()),
        .pPoolSizes = poolSizes.data()
    };

    m_DescriptorPool = vk::raii::DescriptorPool(logicaldevice, poolInfo);

    // create a descriptor set layout with update after-bind flags
    vk::DescriptorSetLayoutBinding storageBufferBinding{
        .binding = 0,
        .descriptorType = vk::DescriptorType::eStorageBuffer,
        .descriptorCount = MAX_BINDLESS_RESOURCES,
        .stageFlags = vk::ShaderStageFlagBits::eAllGraphics | vk::ShaderStageFlagBits::eCompute,
        .pImmutableSamplers = nullptr
    };

    vk::DescriptorSetLayoutBinding textureBinding{
        .binding = 1,
        .descriptorType = vk::DescriptorType::eCombinedImageSampler,
        .descriptorCount = MAX_BINDLESS_RESOURCES,
        .stageFlags = vk::ShaderStageFlagBits::eAllGraphics,
        .pImmutableSamplers = nullptr
    };

    std::array<vk::DescriptorSetLayoutBinding, 2> bindings = {storageBufferBinding, textureBinding};

    std::array<vk::DescriptorBindingFlags, 2> bindingFlags = {
        vk::DescriptorBindingFlagBits::eUpdateAfterBind | vk::DescriptorBindingFlagBits::ePartiallyBound,
        vk::DescriptorBindingFlagBits::eUpdateAfterBind | vk::DescriptorBindingFlagBits::ePartiallyBound};

    vk::DescriptorSetLayoutBindingFlagsCreateInfo extendedInfo{
        .bindingCount = static_cast<uint32_t>(bindingFlags.size()),
        .pBindingFlags = bindingFlags.data()
    };

    vk::DescriptorSetLayoutCreateInfo layoutInfo{
        .pNext = &extendedInfo,
        .flags = vk::DescriptorSetLayoutCreateFlagBits::eUpdateAfterBindPool,
        .bindingCount = static_cast<uint32_t>(bindings.size()),
        .pBindings = bindings.data()
    };
    m_DescriptorSetLayout = vk::raii::DescriptorSetLayout(logicaldevice, layoutInfo);

    vk::DescriptorSetAllocateInfo allocInfo{
        .descriptorPool = *m_DescriptorPool,
        .descriptorSetCount = 1,
        .pSetLayouts = &(*m_DescriptorSetLayout)
    };

    vk::raii::DescriptorSets sets(logicaldevice, allocInfo);
    m_GlobalDescriptorSet = std::move(sets.front());
}

u32 VulkanDescriptorManager::RegisterBuffer(vk::raii::Device& device, vk::Buffer buffer, vk::DeviceSize size)
{
    u32 bindingIndex = m_NextBufferIndex++;

    vk::DescriptorBufferInfo bufferInfo{buffer, 0, size};
    vk::WriteDescriptorSet descriptorWrite{
        .dstSet = *m_GlobalDescriptorSet,
        .dstBinding = 0,                 // binding for storage buffers
        .dstArrayElement = bindingIndex, // next available array element
        .descriptorCount = 1,
        .descriptorType = vk::DescriptorType::eStorageBuffer,
        .pImageInfo = nullptr,
        .pBufferInfo = &bufferInfo,
        .pTexelBufferView = nullptr
    };

    std::array<vk::WriteDescriptorSet, 1> writes = {descriptorWrite};
    device.updateDescriptorSets({descriptorWrite}, nullptr);
    return bindingIndex;
}
u32 VulkanDescriptorManager::BindTexture(vk::raii::Device& device, vk::ImageView imageView,
                                          vk::Sampler sampler)
{
    u32 slot = m_NextTextureIndex++;

    vk::DescriptorImageInfo imageInfo{
        .sampler = sampler,
        .imageView = imageView,
        .imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal
    };

    vk::WriteDescriptorSet descriptorWrite{
        .dstSet = *m_GlobalDescriptorSet,
        .dstBinding = 1,
        .dstArrayElement = slot,
        .descriptorCount = 1,
        .descriptorType = vk::DescriptorType::eCombinedImageSampler,
        .pImageInfo = &imageInfo,
        .pBufferInfo = nullptr,
        .pTexelBufferView = nullptr
    };
    device.updateDescriptorSets({descriptorWrite}, nullptr);
    
    return slot;
}

void VulkanDescriptorManager::cleanup()
{
    m_GlobalDescriptorSet = nullptr;
    m_DescriptorSetLayout = nullptr;
    m_DescriptorPool = nullptr;
}
} // namespace UHE::RHI::VULKAN
