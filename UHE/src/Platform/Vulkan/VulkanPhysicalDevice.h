#pragma once
#include <vulkan/vulkan_raii.hpp>
#include "VulkanInstance.h"


namespace UHE::RHI {
class VulkanPhysicalDevice {
   public:
     VulkanPhysicalDevice() = default;
     VulkanPhysicalDevice(const VulkanPhysicalDevice &) = delete;
     VulkanPhysicalDevice &operator=(const VulkanPhysicalDevice &) = delete;
     
    
     void initPhysicalDevice(instance_vk &instance);
     vk::raii::PhysicalDevice &getPhysicalDevice() { return m_physicalDevice; }
     void GetLogicalDeviceInfo(u32 &vendorID, u32 &deviceID) const {
       vendorID = m_physicalDevice.getProperties().vendorID;
       deviceID = m_physicalDevice.getProperties().deviceID;
     }

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

