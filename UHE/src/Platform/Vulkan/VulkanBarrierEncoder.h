#pragma once
#include <span>
#include <vulkan/vulkan_raii.hpp>
#include "Platform/Vulkan/VulkanTypes.h"

namespace UHE::RHI::VULKAN
{
class VulkanContext;

// One image transition. The subresource range is supplied by the caller so this
// encoder stays independent of the render graph.
struct ImageBarrier
{
    vk::Image Image = nullptr;
    vk::ImageSubresourceRange Range{};
    ImageState Old = ImageState::Undefined;
    ImageState Next = ImageState::Undefined;
    Stage SrcStage = Stage::None;
    Stage DstStage = Stage::None;
    Access SrcAccess = Access::None;
    Access DstAccess = Access::None;
};

struct BufferBarrier
{
    vk::Buffer Buffer = nullptr;
    vk::DeviceSize Offset = 0;
    vk::DeviceSize Size = VK_WHOLE_SIZE;
    Stage SrcStage = Stage::None;
    Stage DstStage = Stage::None;
    Access SrcAccess = Access::None;
    Access DstAccess = Access::None;
};

struct GlobalBarrier
{
    Stage SrcStage = Stage::None;
    Stage DstStage = Stage::None;
    Access SrcAccess = Access::None;
    Access DstAccess = Access::None;
};

// Translates engine barriers into vkCmdPipelineBarrier2 (Sync2) or
// vkCmdPipelineBarrier (Legacy). This is the only place that decides which.
class VulkanBarrierEncoder
{
public:
    VulkanBarrierEncoder() = default;
    VulkanBarrierEncoder(const VulkanBarrierEncoder&) = delete;
    VulkanBarrierEncoder operator=(const VulkanBarrierEncoder&) = delete;
    ~VulkanBarrierEncoder() = default;

    void Init(VulkanContext* context);
    void Shutdown();

    void Encode(vk::CommandBuffer commandBuffer, std::span<const ImageBarrier> imageBarriers,
                std::span<const BufferBarrier> bufferBarriers, const GlobalBarrier* globalBarrier);

private:
    VulkanContext* m_Context = nullptr;
};

} // namespace UHE::RHI::VULKAN
