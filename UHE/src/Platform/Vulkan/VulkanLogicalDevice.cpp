#define VK_NO_PROTOTYPES
#include "uhepch.h"
#include "VulkanLogicalDevice.h"
#include <volk.h>
#include "VulkanInstance.h"
#include "VulkanPhysicalDevice.h"

namespace UHE::RHI::VULKAN
{
void VulkanLogicalDevice::initialize(VulkanPhysicalDevice& physicalDevice, VkSurfaceKHR surface,
                                     VulkanInstance& instance)
{
    const auto& phyDevice = physicalDevice.getPhysicalDevice();
    std::vector<vk::QueueFamilyProperties> queueFamilies = phyDevice.getQueueFamilyProperties();
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

    vk::StructureChain<vk::PhysicalDeviceFeatures2, vk::PhysicalDeviceVulkan11Features, vk::PhysicalDeviceVulkan12Features, vk::PhysicalDeviceVulkan13Features,
                       vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>
        featureChain;

    auto& features2 = featureChain.get<vk::PhysicalDeviceFeatures2>();
    auto& vulkan11Features = featureChain.get<vk::PhysicalDeviceVulkan11Features>();
    auto& vulkan12Features = featureChain.get<vk::PhysicalDeviceVulkan12Features>();
    auto& vulkan13Features = featureChain.get<vk::PhysicalDeviceVulkan13Features>();
    auto& dynamicStateFeatures = featureChain.get<vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>();

    features2.features.samplerAnisotropy = VK_TRUE;
    features2.features.independentBlend = VK_TRUE;

    vulkan11Features.shaderDrawParameters = VK_TRUE;

    vulkan13Features.dynamicRendering = VK_TRUE;
    vulkan13Features.synchronization2 = VK_TRUE;

    dynamicStateFeatures.extendedDynamicState = VK_TRUE;

    vulkan12Features.descriptorBindingStorageBufferUpdateAfterBind = VK_TRUE;
    vulkan12Features.descriptorBindingPartiallyBound = VK_TRUE;
    vulkan12Features.descriptorBindingSampledImageUpdateAfterBind = VK_TRUE;
    vulkan12Features.shaderSampledImageArrayNonUniformIndexing = VK_TRUE;

    float queuePriority = 1.0f;
    vk::DeviceQueueCreateInfo deviceQueueCreateInfo{
        .flags = {},
        .queueFamilyIndex = m_graphicsQueueFamilyIndex,
        .queueCount = 1,
        .pQueuePriorities = &queuePriority
    };

    vk::DeviceCreateInfo deviceCreateInfo{
        .pNext = &featureChain.get<vk::PhysicalDeviceFeatures2>(),
        .flags = {},
        .queueCreateInfoCount = 1,
        .pQueueCreateInfos = &deviceQueueCreateInfo,
        .enabledLayerCount = 0,
        .ppEnabledLayerNames = nullptr,
        .enabledExtensionCount = static_cast<u32>(requiredDeviceExtension.size()),
        .ppEnabledExtensionNames = requiredDeviceExtension.data(),
        .pEnabledFeatures = nullptr
    };

    m_logicalDevice = vk::raii::Device(phyDevice, deviceCreateInfo);
    volkLoadDevice(*m_logicalDevice);
    m_graphicsQueue = vk::raii::Queue(m_logicalDevice, m_graphicsQueueFamilyIndex, 0);

    VmaVulkanFunctions vulkanFunctions{
        .vkGetInstanceProcAddr = vkGetInstanceProcAddr,
        .vkGetDeviceProcAddr = vkGetDeviceProcAddr
    };

    VmaAllocatorCreateInfo allocatorCreateInfo{
        .flags = 0,
        .physicalDevice = *physicalDevice.getPhysicalDevice(),
        .device = *m_logicalDevice,
        .preferredLargeHeapBlockSize = 0,
        .pAllocationCallbacks = nullptr,
        .pDeviceMemoryCallbacks = nullptr,
        .pHeapSizeLimit = nullptr,
        .pVulkanFunctions = &vulkanFunctions,
        .instance = *instance.getInstance(),
        .vulkanApiVersion = VK_API_VERSION_1_3
    };

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
