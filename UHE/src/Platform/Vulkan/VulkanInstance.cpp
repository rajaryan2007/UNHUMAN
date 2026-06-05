#define VK_NO_PROTOTYPES
#define VOLK_IMPLEMENTATION
#include <volk.h>
#include "uhepch.h"
#include "VulkanInstance.h"



std::vector<const char*>validationLayers = { "VK_LAYER_KHRONOS_validation" };
#ifdef NDEBUG
constexpr bool enableValidationLayers = false;
#else
constexpr bool enableValidationLayers = true;
#endif

namespace UHE::RHI::VULKAN {

    VulkanInstance::VulkanInstance()
        : m_context(), m_instance(nullptr), m_debugMessenger(nullptr)
    {
        if (volkInitialize() != VK_SUCCESS) {
            throw std::runtime_error("Failed to initialize Volk!");
        }
        if (!glfwVulkanSupported()) {
            VG_CORE_ERROR("Vulkan is not supported on this system!");
        }
    }

    void VulkanInstance::initialize()
    {
        UHE_PROFILE_FUNCTION();
        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "Unhuman Engine";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 3, 0);
        appInfo.pEngineName = "UHE Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_3;

        std::vector<const char*> requiredLayer;
        if (enableValidationLayers)
        {
            requiredLayer.assign(validationLayers.begin(), validationLayers.end());
        }   

        auto layerProperties = m_context.enumerateInstanceLayerProperties();
        if (std::ranges::any_of(requiredLayer, [&layerProperties](
            auto const& requiredLayer) {
                return std::ranges::none_of(
                    layerProperties, [requiredLayer](auto const& layerProperties) {
                        return strcmp(layerProperties.layerName, requiredLayer) == 0;
                    });
            }))
        {
            throw std::runtime_error("Validation layers requested, but not available!");
        }

        auto requiredExtensions = getRequiredExtensions();

        auto requiredExtensionsProperties = m_context.enumerateInstanceExtensionProperties();
        if (std::ranges::any_of(
                requiredExtensions,
                [&requiredExtensionsProperties](auto const &requiredExtension) {
                  return std::ranges::none_of(
                      requiredExtensionsProperties,
                      [requiredExtension](auto const &extensionProperties) {
                        return strcmp(extensionProperties.extensionName,
                                      requiredExtension) == 0;
                      });
                })) {
          throw std::runtime_error("Required extensions are not available!");
        };

        vk::DebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
        if (enableValidationLayers)
        {
          debugCreateInfo.messageSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |vk::DebugUtilsMessageSeverityFlagBitsEXT::eError;

          debugCreateInfo.messageType = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance;
          debugCreateInfo.pfnUserCallback = debugCallback;
        }

        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;
        createInfo.enabledLayerCount = static_cast<uint32_t>(requiredLayer.size()); 
        createInfo.ppEnabledLayerNames = requiredLayer.data();
        createInfo.enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size());
        createInfo.ppEnabledExtensionNames = requiredExtensions.empty() ? nullptr : requiredExtensions.data();
        if (enableValidationLayers)
        {
          createInfo.pNext = &debugCreateInfo;
        }

        m_instance = vk::raii::Instance(m_context, createInfo);

        volkLoadInstance(*m_instance);
        if (enableValidationLayers)
        {
          m_debugMessenger = vk::raii::DebugUtilsMessengerEXT(m_instance, debugCreateInfo);
        }
    }

    std::vector<const char*> VulkanInstance::getRequiredExtensions()
    {
        u32 glfwExtensionCount = 0;
        auto glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
        std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

        if (enableValidationLayers)
        {
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }
        return extensions;
    }

    vk::raii::Instance& VulkanInstance::getInstance()
    { 
        return m_instance;
    }

    VKAPI_ATTR vk::Bool32 VKAPI_CALL VulkanInstance::debugCallback(
        vk::DebugUtilsMessageSeverityFlagBitsEXT severity,
        vk::DebugUtilsMessageTypeFlagsEXT type,
        const vk::DebugUtilsMessengerCallbackDataEXT *pCallbackData, void *)
    {
        if (severity & vk::DebugUtilsMessageSeverityFlagBitsEXT::eError ||
            severity & vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning)
        {
            VG_CORE_ERROR("Validation layer: {0}", pCallbackData->pMessage);
        }
        else
        {
            VG_CORE_INFO("Validation layer: {0}", pCallbackData->pMessage);
        }
        return vk::False;
    }

    void VulkanInstance::cleanup() 
    {
	  UHE_PROFILE_FUNCTION();
	
    }
  } // namespace UHE
