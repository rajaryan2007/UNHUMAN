#pragma once
#include <optional>
#include <vulkan/vulkan_raii.hpp>
#include "VulkanInstance.h"
#include "vulkan/vulkan.hpp"

namespace UHE::RHI::VULKAN
{
struct QueueFamilyIndices
{
    std::optional<uint32_t> graphicsFamily;
    bool isComplete() const { return graphicsFamily.has_value(); }
};

class VulkanPhysicalDevice
{
public:
    VulkanPhysicalDevice() = default;
    VulkanPhysicalDevice(const VulkanPhysicalDevice&) = delete;
    VulkanPhysicalDevice& operator=(const VulkanPhysicalDevice&) = delete;

    void initPhysicalDevice(VulkanInstance& instance);
    [[nodiscard]] vk::raii::PhysicalDevice& getPhysicalDevice() { return m_physicalDevice; }
    [[nodiscard]] const QueueFamilyIndices& getQueueFamilyIndices() const { return m_queueFamilyIndices; }
    [[nodiscard]] i32 RateDevice(const vk::raii::PhysicalDevice& device) const;
    [[nodiscard]] bool IsDeviceSuitable(const vk::raii::PhysicalDevice& device) const;
    void GetLogicalDeviceInfo(u32& vendorID, u32& deviceID) const
    {
        vendorID = m_physicalDevice.getProperties().vendorID;
        deviceID = m_physicalDevice.getProperties().deviceID;
    }

private:
    vk::PhysicalDevice FindPhysicalDevice();

private:
    vk::raii::PhysicalDevice m_physicalDevice = nullptr;
    QueueFamilyIndices m_queueFamilyIndices;
    std::vector<const char*> requiredDeviceExtension = {
        vk::KHRSwapchainExtensionName,           vk::KHRSpirv14ExtensionName,
        vk::KHRSynchronization2ExtensionName,    vk::KHRCreateRenderpass2ExtensionName,
        vk::KHRShaderFloatControlsExtensionName, vk::KHRDynamicRenderingExtensionName};
};
} // namespace UHE::RHI::VULKAN
