#pragma once

#include <vulkan/vulkan_raii.hpp>
#include <GLFW/glfw3.h>
namespace UHE::RHI {
	class instance_vk {
	  public:
        instance_vk();
		instance_vk(const instance_vk &) = delete;
		instance_vk &operator=(const instance_vk &) = delete;
		void initialize();
        std::vector<const char *> getRequiredExtensions();
        static VKAPI_ATTR vk::Bool32 VKAPI_CALL debugCallback(
                    vk::DebugUtilsMessageSeverityFlagBitsEXT severity,
                    vk::DebugUtilsMessageTypeFlagsEXT type,
                    const vk::DebugUtilsMessengerCallbackDataEXT *pCallbackData,
                    void *);
        void cleanup();
        vk::raii::Instance &getInstance();
      private:
        vk::raii::Context m_context;      
        vk::raii::Instance m_instance = nullptr;
        vk::raii::DebugUtilsMessengerEXT m_debugMessenger = nullptr;
    };
}
