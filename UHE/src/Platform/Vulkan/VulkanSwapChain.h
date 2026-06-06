#pragma once
#include <GLFW/glfw3.h>
#include <vulkan/vulkan_raii.hpp>
#include "UHE/RHI/RHISwapChain.h"

namespace UHE::RHI::VULKAN
{
class VulkanSwapChain : RHISwapChain
{
public:
    VulkanSwapChain() = default;
    VulkanSwapChain(const VulkanSwapChain&) = delete;
    VulkanSwapChain& operator=(const VulkanSwapChain&) = delete;
    void createSwapChain(vk::Device& device, vk::raii::PhysicalDevice& physicalDevice, vk::raii::SurfaceKHR& surface,
                         GLFWwindow* window);

    void cleanupSwapChain();

    virtual void AcquireNextImage() override;
    virtual void Present() override;
    virtual void ResizeSwapchain(u32 width, u32 height) override;
    virtual TextureHandle GetSwapchainImage() override;
    virtual TextureFormat GetSwapchainFormat() override;

    inline vk::raii::SwapchainKHR& GetSwapchain() { return swapChain; }
    inline const std::vector<vk::Image>& GetImages() const { return swapChainImages; }

    inline vk::raii::ImageView& GetImageView(u32 index) { return swapChainImageViews[index]; }
    inline const vk::Extent2D& GetExtent() const { return swapChainExtent; }
    inline vk::SurfaceFormatKHR& GetSurfaceFormat() { return swapChainSurfaceFormat; }

private:
    vk::Extent2D chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities, GLFWwindow* window);

    u32 chooseSwapMinImageCount(vk::SurfaceCapabilitiesKHR const& surfaceCapablities);

    vk::SurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats);

    vk::PresentModeKHR chooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes);

private:
    vk::raii::SwapchainKHR swapChain = nullptr;
    std::vector<vk::Image> swapChainImages;
    vk::SurfaceFormatKHR swapChainSurfaceFormat;
    vk::Extent2D swapChainExtent;
    std::vector<vk::raii::ImageView> swapChainImageViews;
    vk::raii::Sampler textureSampler = nullptr;
    vk::raii::ImageView textureImageView = nullptr;
};

} // namespace UHE::RHI::VULKAN
