#pragma once
#include <vulkan/vulkan_raii.hpp>


namespace UHE::RHI::VULKAN {
class VulkanDevice;
class VulkanDescriptorManager
{
public:
    VulkanDescriptorManager() = default;
    VulkanDescriptorManager(const VulkanDescriptorManager &) = delete;
    VulkanDescriptorManager &
    operator=(const VulkanDescriptorManager &) = delete;

    void init(VulkanDevice& device);
    u32 RegisterBuffer(vk::raii::Device &device, vk::Buffer buffer,
                        vk::DeviceSize size);
    void cleanup();

    vk::DescriptorSetLayout GetLayoutHandle() const {
      return *m_DescriptorSetLayout;
    }
    vk::DescriptorSet GetSetHandle() const { return *m_GlobalDescriptorSet; }
  private:
    vk::raii::DescriptorPool m_DescriptorPool = nullptr;
    vk::raii::DescriptorSetLayout m_DescriptorSetLayout = nullptr;
    vk::raii::DescriptorSet m_GlobalDescriptorSet = nullptr;

    uint32_t m_NextBufferIndex = 0;
    uint32_t m_NextTextureIndex = 0;
};
} // namespace UHE::RHI


