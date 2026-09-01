#pragma once
#include <vulkan/vulkan_raii.hpp>
#include "UHE/RHI/RHITypes.h"
#include "vulkan/vulkan.hpp"
namespace UHE::RHI::VULKAN
{

struct VulkanContext;
class VulkanRenderPass
{
public:
    VulkanRenderPass() = default;
    ~VulkanRenderPass() = default;
    VulkanRenderPass(VulkanRenderPass&) = delete;
    VulkanRenderPass operator=(VulkanRenderPass&) = delete;
    void Init(const VulkanContext& ctx, const UHE::RHI::RenderPassDesc& desc);
    [[nodiscard]] vk::RenderPass GetRenderPass() const { return *m_RenderPass; }
    // TODO
    // implement the RenderPass Builder Feature so it can run better in old hardware

private:
    vk::raii::RenderPass m_RenderPass{nullptr};
};
} // namespace UHE::RHI::VULKAN
