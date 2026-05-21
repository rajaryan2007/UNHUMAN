#pragma once
#include <vulkan/vulkan_raii.hpp>
#include "VulkanInstance.h"


namespace UHE::RHI {
   class PhysicalDevice {
   public:
     PhysicalDevice() = default;
     PhysicalDevice(const PhysicalDevice &) = delete;
     PhysicalDevice &operator=(const PhysicalDevice &) = delete;
     
     

     void initPhysicalDevice(instance_vk &instance);
     vk::raii::PhysicalDevice &getPhysicalDevice() { return m_physicalDevice; }

   private:
     vk::raii::PhysicalDevice m_physicalDevice;
     std::vector<const char *> requiredDeviceExtension = {
         vk::KHRSwapchainExtensionName,
         vk::KHRSpirv14ExtensionName,
         vk::KHRSynchronization2ExtensionName,
         vk::KHRCreateRenderpass2ExtensionName,
         vk::KHRShaderFloatControlsExtensionName,
         vk::KHRDynamicRenderingExtensionName};
   };
} // namespace UHE

