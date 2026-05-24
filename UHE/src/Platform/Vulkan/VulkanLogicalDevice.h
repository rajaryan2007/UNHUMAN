#pragma once
#include <glfw/glfw3.h>
#include <vulkan/vulkan_raii.hpp>
#include <vk_mem_alloc.h>



namespace UHE::RHI {
  class VulkanPhysicalDevice;
  class VulkanInstance;

  class VulkanLogicalDevice {
  public:
    VulkanLogicalDevice() = default;
    VulkanLogicalDevice(const VulkanLogicalDevice &) = delete;
    VulkanLogicalDevice &operator=(const VulkanLogicalDevice &) = delete;

    void initialize(VulkanPhysicalDevice& physicalDevice, VkSurfaceKHR surface, VulkanInstance& instance);
    void CreateSurface(VulkanInstance& instance, GLFWwindow *window);
    void cleanup();

    VmaAllocator &getAllocator() 
    {
      return m_allocator;
    };

    inline const vk::raii::Device &getLogicalDevice() { return m_logicalDevice; }

   private:
    u32 m_graphicsQueueFamilyIndex{0};
    vk::raii::Device m_logicalDevice = nullptr;
    vk::raii::Queue m_graphicsQueue = nullptr;
    vk::raii::SurfaceKHR surface = nullptr;
    VmaAllocator m_allocator = nullptr;


    std::vector<const char *> requiredDeviceExtension = {
        vk::KHRSwapchainExtensionName,
        vk::KHRSpirv14ExtensionName,
        vk::KHRSynchronization2ExtensionName,
        vk::KHRCreateRenderpass2ExtensionName,
        vk::KHRShaderFloatControlsExtensionName,
        vk::KHRDynamicRenderingExtensionName};
  };
}

