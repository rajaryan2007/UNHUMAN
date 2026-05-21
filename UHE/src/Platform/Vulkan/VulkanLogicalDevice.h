#pragma once
#include <glfw/glfw3.h>
#include <vulkan/vulkan_raii.hpp>
#include <vk_mem_alloc.h>



namespace UHE::RHI {
  class PhysicalDevice;
  class instance_vk; 

  class VkLogicalDevice {
  public:
    VkLogicalDevice() = default;
    VkLogicalDevice(const VkLogicalDevice &) = delete;
    VkLogicalDevice &operator=(const VkLogicalDevice &) = delete;

    void initialize(PhysicalDevice& physicalDevice, VkSurfaceKHR surface, instance_vk& instance);
    void CreateSurface(instance_vk& instance, GLFWwindow *window);
    void cleanup();

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

