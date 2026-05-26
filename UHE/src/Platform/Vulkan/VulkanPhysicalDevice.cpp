#include "uhepch.h"
#include "VulkanPhysicalDevice.h"



namespace UHE::RHI::VULKAN {
void VulkanPhysicalDevice::initPhysicalDevice(VulkanInstance &m_Vkinstance) 
  {
  auto const &m_instance = m_Vkinstance.getInstance();
     std::vector<vk::raii::PhysicalDevice> devices =   m_instance.enumeratePhysicalDevices();
     if (devices.empty()) {
         throw std::runtime_error("Failed to find a GPU with Vulkan support!");
     }
     m_physicalDevice = devices[0];

      const auto deviter = std::ranges::find_if(devices,
        [&](auto const &device) 
        {
             bool supportsVulkan1_3 =device.getProperties().apiVersion >= VK_API_VERSION_1_3;
             auto queueFamilies = device.getQueueFamilyProperties();
             bool supportsGraphics = std::ranges::any_of(
                 queueFamilies, [](auto const &qfp) {return !!(qfp.queueFlags &vk::QueueFlagBits::eGraphics); });

             auto availableExtensions = device.enumerateDeviceExtensionProperties();
             bool supportsAllRequiredExtensions =
                 std::ranges::all_of(
                     requiredDeviceExtension,
                     [availableExtensions](const char* requiredDeviceExtension) {
                       return std::ranges::any_of(availableExtensions,
                           [requiredDeviceExtension](const auto &extensionProperties) 
                                { return strcmp(extensionProperties.extensionName,requiredDeviceExtension) == 0;});

                });

              auto features = device.template getFeatures2< vk::PhysicalDeviceFeatures2,vk::PhysicalDeviceVulkan13Features,vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>();

              bool supportsRequiredFeatures = features.template get<vk::PhysicalDeviceVulkan13Features>().dynamicRendering &&
                                           features.template get<vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>().extendedDynamicState;

              return supportsVulkan1_3 && supportsGraphics &&  supportsAllRequiredExtensions && supportsRequiredFeatures;

        });
     if (deviter != devices.end()) 
     {
       m_physicalDevice = *deviter;
     }
     else
     {
       throw std::runtime_error("Failed to find suitable GPU with Vulkan support!");
     }
  }
}


