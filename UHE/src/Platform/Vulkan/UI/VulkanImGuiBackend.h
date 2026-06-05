#pragma once
#include "UHE/ImGui/ImGuiLayer.h"

namespace UHE::RHI::VULKAN
{
class VulkanImGuiLayer : public ImGuiLayer
{
    VulkanImGuiLayer() = default;
    ~VulkanImGuiLayer() = default;

    virtual void Begin() override;
    virtual void End() override;
};
} // namespace UHE::RHI::VULKAN
