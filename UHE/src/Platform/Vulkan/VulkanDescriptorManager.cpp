#include "uhepch.h"
#include "VulkanDescriptorManager.h"
#include "VulkanDevice.h"


namespace UHE::RHI {
void VulkanDescriptorManager::init(VulkanDevice &device) 
{
  const auto &logicaldevice = device.getLogicalDevClass().getLogicalDevice();

  const uint32_t MAX_BINDLESS_RESOURCES = 10000;

  std::array<vk::DescriptorPoolSize, 2> poolSizes = {
    vk::DescriptorPoolSize(vk::DescriptorType::eUniformBuffer, MAX_BINDLESS_RESOURCES),
    vk::DescriptorPoolSize(vk::DescriptorType::eCombinedImageSampler, MAX_BINDLESS_RESOURCES)
  };

  vk::DescriptorPoolCreateInfo poolInfo{};
  poolInfo.flags = vk::DescriptorPoolCreateFlagBits::
      eUpdateAfterBind; // alllow updating descriptors after they've been bound
  poolInfo.maxSets =
      MAX_BINDLESS_RESOURCES * 2; // max number of descriptor sets
  poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
  poolInfo.pPoolSizes = poolSizes.data();
  poolInfo.pNext = nullptr;

  m_DescriptorPool = vk::raii::DescriptorPool(logicaldevice, poolInfo);
  
  // create a descriptor set layout with update after-bind flags
  vk::DescriptorSetLayoutBinding storageBufferBinding{};
  storageBufferBinding.binding = 0;
  storageBufferBinding.descriptorType = vk::DescriptorType::eUniformBuffer;
  storageBufferBinding.descriptorCount = MAX_BINDLESS_RESOURCES;
  storageBufferBinding.stageFlags =
      vk::ShaderStageFlagBits::eAllGraphics | vk::ShaderStageFlagBits::eCompute;
 
  vk::DescriptorSetLayoutBinding textureBinding{};
  textureBinding.binding = 1;
  textureBinding.descriptorType = vk::DescriptorType::eCombinedImageSampler;
  textureBinding.descriptorCount = MAX_BINDLESS_RESOURCES;
  textureBinding.stageFlags = vk::ShaderStageFlagBits::eAllGraphics;

  std::array<vk::DescriptorSetLayoutBinding, 2> bindings = {
      storageBufferBinding, textureBinding};

  std::array<vk::DescriptorBindingFlags, 2> bindingFlags = {
    vk::DescriptorBindingFlagBits::eUpdateAfterBind,
    vk::DescriptorBindingFlagBits::eUpdateAfterBind
  };

  vk::DescriptorSetLayoutBindingFlagsCreateInfo extendedInfo{};
  extendedInfo.bindingCount = static_cast<uint32_t>(bindingFlags.size());
  extendedInfo.pBindingFlags = bindingFlags.data();

  vk::DescriptorSetLayoutCreateInfo layoutInfo{};
  layoutInfo.flags =
      vk::DescriptorSetLayoutCreateFlagBits::eUpdateAfterBindPool;
  layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
  layoutInfo.pBindings = bindings.data();
  layoutInfo.pNext = &extendedInfo;
  m_DescriptorSetLayout = vk::raii::DescriptorSetLayout(logicaldevice, layoutInfo);
  

  vk::DescriptorSetAllocateInfo allocInfo{};
  allocInfo.descriptorPool = *m_DescriptorPool;
  allocInfo.descriptorSetCount = 1;
  allocInfo.pSetLayouts = &(*m_DescriptorSetLayout);

  vk::raii::DescriptorSets sets(logicaldevice, allocInfo);
  m_GlobalDescriptorSet = std::move(sets.front());
}

u32 VulkanDescriptorManager::RegisterBuffer(vk::raii::Device &device,
                                             vk::Buffer buffer,
                                             vk::DeviceSize size)
{
  u32 bindingIndex = m_NextBufferIndex++;
    
  vk::DescriptorBufferInfo bufferInfo{buffer, 0, size};
  vk::WriteDescriptorSet descriptorWrite{};
  descriptorWrite.dstSet = *m_GlobalDescriptorSet;
  descriptorWrite.dstBinding = 0; // binding for storage buffers
  descriptorWrite.dstArrayElement =
      bindingIndex; // next available array element
  descriptorWrite.descriptorCount = 1; 
  descriptorWrite.descriptorType = vk::DescriptorType::eStorageBuffer;
  descriptorWrite.pBufferInfo = &bufferInfo;

  device.updateDescriptorSets({descriptorWrite}, nullptr);
  return bindingIndex; 
}



} // namespace UHE::RHI