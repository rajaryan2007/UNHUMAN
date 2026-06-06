#include "uhepch.h"
#include "VulkanSwapChain.h"

namespace UHE::RHI::VULKAN
{
void VulkanSwapChain::createSwapChain(vk::Device& device, vk::raii::PhysicalDevice& physicalDevice,
                                      vk::raii::SurfaceKHR& surface, GLFWwindow* window)
{
    auto SurfaceCapabilities = physicalDevice.getSurfaceCapabilitiesKHR(*surface);
    swapChainExtent = chooseSwapExtent(SurfaceCapabilities, window);
    swapChainSurfaceFormat = chooseSwapSurfaceFormat(physicalDevice.getSurfaceFormatsKHR(*surface));
    auto SurfaceFormats = physicalDevice.getSurfaceFormatsKHR(*surface);

    vk::SwapchainCreateInfoKHR swapChainCreateInfo{};
    swapChainCreateInfo.surface = *surface;
    swapChainCreateInfo.minImageCount = chooseSwapMinImageCount(SurfaceCapabilities);
    swapChainCreateInfo.imageFormat = swapChainSurfaceFormat.format;
    swapChainCreateInfo.imageColorSpace = swapChainSurfaceFormat.colorSpace;
    swapChainCreateInfo.imageExtent = swapChainExtent;
    swapChainCreateInfo.imageArrayLayers = 1;
    swapChainCreateInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;
    swapChainCreateInfo.imageSharingMode = vk::SharingMode::eExclusive;
    swapChainCreateInfo.preTransform = SurfaceCapabilities.currentTransform;
    swapChainCreateInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
    swapChainCreateInfo.presentMode = chooseSwapPresentMode(physicalDevice.getSurfacePresentModesKHR(*surface));
    swapChainCreateInfo.clipped = VK_TRUE;

    swapChain = vk::raii::SwapchainKHR(device, swapChainCreateInfo);
    swapChainImages = swapChain.getImages();
}

vk::Extent2D VulkanSwapChain::chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities, GLFWwindow* window)
{
    if (capabilities.currentExtent.width != 0xFFFFFFFF)
    {
        return capabilities.currentExtent;
    }
    i32 width, height;

    glfwGetFramebufferSize(window, &width, &height);

    return vk::Extent2D(
        std::clamp(static_cast<uint32_t>(width), capabilities.minImageExtent.width, capabilities.maxImageExtent.width),
        std::clamp(static_cast<uint32_t>(height), capabilities.minImageExtent.height,
                   capabilities.maxImageExtent.height));
}

u32 VulkanSwapChain::chooseSwapMinImageCount(vk::SurfaceCapabilitiesKHR const& surfaceCapablities)
{
    auto minImageCount = std::max(3u, surfaceCapablities.minImageCount);
    if ((0 < surfaceCapablities.maxImageCount) && (minImageCount > surfaceCapablities.maxImageCount))
    {
        minImageCount = surfaceCapablities.maxImageCount;
    }
    return minImageCount;
}

vk::PresentModeKHR VulkanSwapChain::chooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes)
{
    assert(std::ranges::any_of(availablePresentModes,
                               [](auto presentMode) { return presentMode == vk::PresentModeKHR::eFifo; }));

    return std::ranges::any_of(availablePresentModes,
                               [](const vk::PresentModeKHR value) { return vk::PresentModeKHR::eMailbox == value; })
               ? vk::PresentModeKHR::eMailbox
               : vk::PresentModeKHR::eFifo;
}

vk::SurfaceFormatKHR VulkanSwapChain::chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats)
{
    assert(!availableFormats.empty());
    const auto formatIt = std::find_if(availableFormats.begin(), availableFormats.end(),
                                       [](const auto& availableFormat)
                                       {
                                           return availableFormat.format == vk::Format::eB8G8R8A8Srgb &&
                                                  availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear;
                                       });
    return formatIt != availableFormats.end() ? *formatIt : availableFormats[0];
}

void VulkanSwapChain::cleanupSwapChain()
{
    if (swapChain != nullptr)
    {
        swapChain = nullptr;
    }
}

void VulkanSwapChain::AcquireNextImage() {}

void VulkanSwapChain::Present() {}

void VulkanSwapChain::ResizeSwapchain(u32 width, u32 height) {}

TextureHandle VulkanSwapChain::GetSwapchainImage() {}

TextureFormat VulkanSwapChain::GetSwapchainFormat() {}
} // namespace UHE::RHI::VULKAN
