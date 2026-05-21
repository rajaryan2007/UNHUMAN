#define VK_NO_PROTOTYPES
#include <volk.h>
#include "uhepch.h"
#include "VulkanLogicalDevice.h"
#include "VulkanPhysicalDevice.h"
#include "VulkanInstance.h"



namespace UHE::RHI {
  void VkLogicalDevice::initialize(PhysicalDevice& physicalDevice, VkSurfaceKHR surface, instance_vk& instance) 
  {
     const auto& phyDevice = physicalDevice.getPhysicalDevice();
     std::vector<vk::QueueFamilyProperties> queueFamilies  =  phyDevice.getQueueFamilyProperties();
     for (size_t i = 0; i < queueFamilies.size(); i++) {
         if (queueFamilies[i].queueFlags & vk::QueueFlagBits::eGraphics) {
             // Check if the queue family supports presentation to the surface
             if (phyDevice.getSurfaceSupportKHR(i, surface)) {
                 // Store the index of the graphics and presentation queue family
                 m_graphicsQueueFamilyIndex = i;
                 break;
             }
         }
     }
     if (queueFamilies.empty())
     {
       throw std::runtime_error("Failed to find a suitable queue family!");
     }

     vk::StructureChain<vk::PhysicalDeviceFeatures2,
                        vk::PhysicalDeviceVulkan13Features,
                        vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT> featureChain;

     auto& features2 = featureChain.get<vk::PhysicalDeviceFeatures2>();
     auto& vulkan13Features = featureChain.get<vk::PhysicalDeviceVulkan13Features>();
     auto& dynamicStateFeatures = featureChain.get<vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>();

     features2.features.samplerAnisotropy = VK_TRUE;

     vulkan13Features.dynamicRendering = VK_TRUE;
     vulkan13Features.synchronization2 = VK_TRUE;
     
     dynamicStateFeatures.extendedDynamicState = VK_TRUE;

     float queuePriority = 1.0f;
     vk::DeviceQueueCreateInfo deviceQueueCreateInfo{};
     deviceQueueCreateInfo.queueFamilyIndex = m_graphicsQueueFamilyIndex; // Use the index of the graphics
     deviceQueueCreateInfo.queueCount = 1;
     deviceQueueCreateInfo.pQueuePriorities = &queuePriority;
     
     vk::DeviceCreateInfo deviceCreateInfo{};
     deviceCreateInfo.pNext = &featureChain.get<vk::PhysicalDeviceFeatures2>();
     deviceCreateInfo.queueCreateInfoCount = 1;
     deviceCreateInfo.pQueueCreateInfos = &deviceQueueCreateInfo;
     deviceCreateInfo.enabledExtensionCount = static_cast<u32>(requiredDeviceExtension.size());
     deviceCreateInfo.ppEnabledExtensionNames = requiredDeviceExtension.data();
     
     m_logicalDevice = vk::raii::Device(phyDevice, deviceCreateInfo);
     volkLoadDevice(*m_logicalDevice);
     m_graphicsQueue = vk::raii::Queue(m_logicalDevice, m_graphicsQueueFamilyIndex, 0);

     VmaVulkanFunctions vulkanFunctions{};
     vulkanFunctions.vkGetInstanceProcAddr = vkGetInstanceProcAddr;
     vulkanFunctions.vkGetDeviceProcAddr = vkGetDeviceProcAddr;

     VmaAllocatorCreateInfo allocatorCreateInfo = {};
     allocatorCreateInfo.vulkanApiVersion = VK_API_VERSION_1_3;
     allocatorCreateInfo.physicalDevice = *physicalDevice.getPhysicalDevice();
     allocatorCreateInfo.device = *m_logicalDevice;
     allocatorCreateInfo.instance = *instance.getInstance();
     allocatorCreateInfo.pVulkanFunctions = &vulkanFunctions;

     if (vmaCreateAllocator(&allocatorCreateInfo, &m_allocator) != VK_SUCCESS)
     {
       throw std::runtime_error("Failed to create VMA allocator!");
     }
  }

  void VkLogicalDevice::CreateSurface(instance_vk& instance, GLFWwindow* window) 
  {
    auto const &m_instance = instance.getInstance();

    VkSurfaceKHR _surface;
    if (glfwCreateWindowSurface(*m_instance, window, nullptr, &_surface) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create window surface!");
    }
    surface = vk::raii::SurfaceKHR(m_instance, _surface);
  }


  void VkLogicalDevice::cleanup() 
  {
    vmaDestroyAllocator(m_allocator);
    m_logicalDevice = nullptr;
  }
}



