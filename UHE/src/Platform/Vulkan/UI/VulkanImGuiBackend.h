#pragma once
#include <vulkan/vulkan_raii.hpp>
#include <memory>
#include "UHE/ImGui/ImGuiLayer.h"
namespace UHE::RHI::VULKAN
{
class VulkanDevice;

class VulkanImGuiLayer : public ImGuiLayer
{
public:
    VulkanImGuiLayer(VulkanDevice* device);
    ~VulkanImGuiLayer() override = default;

    virtual void OnAttach() override;
    virtual void OnDetach() override;
    virtual void Begin() override;
    virtual void End() override;

private:
    VulkanDevice* m_Device;
    std::unique_ptr<vk::raii::DescriptorPool> m_DescriptorPool;
};
} // namespace UHE::RHI::VULKAN
