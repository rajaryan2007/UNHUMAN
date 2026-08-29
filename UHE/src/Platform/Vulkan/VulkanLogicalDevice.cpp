#define VK_NO_PROTOTYPES
#include "uhepch.h"
#include "VulkanLogicalDevice.h"
#include <volk.h>
#include "VulkanExtensionCheck.h"
#include "VulkanInstance.h"
#include "VulkanPhysicalDevice.h"
namespace UHE::RHI::VULKAN
{
void VulkanLogicalDevice::initialize(VulkanPhysicalDevice& physicalDevice, VkSurfaceKHR surface,
                                     VulkanInstance& instance, VulkanExtensionCheck& CheckExtens)
{
    const auto& phyDevice = physicalDevice.getPhysicalDevice();
    std::vector<vk::QueueFamilyProperties> queueFamilies = phyDevice.getQueueFamilyProperties();
    m_graphicsQueueFamilyIndex = static_cast<u32>(-1);
    for (size_t i = 0; i < queueFamilies.size(); i++)
    {
        if (queueFamilies[i].queueFlags & vk::QueueFlagBits::eGraphics)
        {
            // Check if the queue family supports presentation to the surface
            if (phyDevice.getSurfaceSupportKHR(i, surface))
            {
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

    CheckExtens.TickTheAvailableExtension(phyDevice);
    CheckExtens.QuerySupportedFeatures(phyDevice);
    auto deviceExtensions = CheckExtens.GetEnabledDeviceExtensions();

    float queuePriority = 1.0f;
    vk::DeviceQueueCreateInfo deviceQueueCreateInfo{.flags = {},
                                                    .queueFamilyIndex = m_graphicsQueueFamilyIndex,
                                                    .queueCount = 1,
                                                    .pQueuePriorities = &queuePriority};

    vk::DeviceCreateInfo deviceCreateInfo{.pNext = CheckExtens.BuildDeviceFeatureChain(),
                                          .flags = {},
                                          .queueCreateInfoCount = 1,
                                          .pQueueCreateInfos = &deviceQueueCreateInfo,
                                          .enabledLayerCount = 0,
                                          .ppEnabledLayerNames = nullptr,
                                          .enabledExtensionCount = static_cast<u32>(deviceExtensions.size()),
                                          .ppEnabledExtensionNames = deviceExtensions.data(),
                                          .pEnabledFeatures = nullptr};

    m_logicalDevice = vk::raii::Device(phyDevice, deviceCreateInfo);
    volkLoadDevice(*m_logicalDevice);
    VULKAN_HPP_DEFAULT_DISPATCHER.init(*instance.getInstance(), *m_logicalDevice);
    m_graphicsQueue = vk::raii::Queue(m_logicalDevice, m_graphicsQueueFamilyIndex, 0);

    VmaVulkanFunctions vulkanFunctions{.vkGetInstanceProcAddr = vkGetInstanceProcAddr,
                                       .vkGetDeviceProcAddr = vkGetDeviceProcAddr};

    VmaAllocatorCreateInfo allocatorCreateInfo{.flags = 0,
                                               .physicalDevice = *physicalDevice.getPhysicalDevice(),
                                               .device = *m_logicalDevice,
                                               .preferredLargeHeapBlockSize = 0,
                                               .pAllocationCallbacks = nullptr,
                                               .pDeviceMemoryCallbacks = nullptr,
                                               .pHeapSizeLimit = nullptr,
                                               .pVulkanFunctions = &vulkanFunctions,
                                               .instance = *instance.getInstance(),
                                               .vulkanApiVersion = VK_API_VERSION_1_3};

    if (vmaCreateAllocator(&allocatorCreateInfo, &m_allocator) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create VMA allocator!");
    }
}

void VulkanLogicalDevice::CreateSurface(VulkanInstance& instance, GLFWwindow* window)
{
    auto const& m_instance = instance.getInstance();

    VkSurfaceKHR _surface;
    if (glfwCreateWindowSurface(*m_instance, window, nullptr, &_surface) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create window surface!");
    }
    surface = vk::raii::SurfaceKHR(m_instance, _surface);
}

void VulkanLogicalDevice::cleanup()
{
    vmaDestroyAllocator(m_allocator);
    m_logicalDevice = nullptr;
}
} // namespace UHE::RHI::VULKAN
