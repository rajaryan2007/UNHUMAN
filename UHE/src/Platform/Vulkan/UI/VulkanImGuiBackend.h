#pragma once
#include <memory>
#include <vulkan/vulkan_raii.hpp>
#include "UHE/ImGui/ImGuiLayer.h"
namespace UHE::RHI::VULKAN
{
class VulkanDevice;

class VulkanImGuiLayer final : public ImGuiLayer
{
public:
    VulkanImGuiLayer(VulkanDevice* device);
    ~VulkanImGuiLayer() override = default;

    void OnAttach() override;
    void OnDetach() override;
    void Begin() override;
    void End() override;

private:
    VulkanDevice* m_Device;
    std::unique_ptr<vk::raii::DescriptorPool> m_DescriptorPool;
};
} // namespace UHE::RHI::VULKAN
