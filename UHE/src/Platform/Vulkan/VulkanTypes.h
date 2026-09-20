#pragma once
#include <vulkan/vulkan_raii.hpp>
#include "UHE/RHI/RHITypes.h"

namespace UHE::RHI::VULKAN
{
vk::Format MapTextureFormat(TextureFormat format);
vk::PrimitiveTopology MapTopology(PrimitiveTopology topology);
vk::Format ShaderDataTypeToVulkanFormat(ShaderDataType type);
vk::Format ToVkFormat(TextureFormat format);
vk::SampleCountFlagBits ToVkSample(u32 sampleCount);
vk::AttachmentStoreOp ToVkStoreOp(StoreOp storeOp);
vk::AttachmentLoadOp ToVkLoadOp(LoadOp loadOp);
vk::ImageLayout ToVkImageLayout(TextureUsage usage);
vk::DescriptorType ToVkDescriptorType(BufferUsageFlags usage);

// --- Synchronization vocabulary ---------------------------------------------
// Engine-level intent for barriers and submits. The encoders translate these
// into v2 or v1 Vulkan, so nothing above this layer writes VkPipelineStageFlags2.
enum class Stage : u64
{
    None = 0,
    TopOfPipe,
    Transfer,
    Compute,
    Vertex,
    Fragment,
    ColorOutput,
    DepthEarly,
    DepthLate,
    BottomOfPipe,
};

enum class Access : u64
{
    None = 0,
    TransferRead,
    TransferWrite,
    ShaderRead,
    ShaderWrite,
    ColorWrite,
    DepthWrite,
    MemoryRead,
    MemoryWrite,
};

enum class ImageState : u8
{
    Undefined = 0,
    ColorAttachment,
    DepthAttachment,
    ShaderRead,
    TransferSrc,
    TransferDst,
    Present,
    Storage,
};

struct SemaphoreWait
{
    vk::Semaphore Semaphore = nullptr;
    Stage WaitStage = Stage::None;
    u64 Value = 0;
};

struct SemaphoreSignal
{
    vk::Semaphore Semaphore = nullptr;
    u64 Value = 0;
};

vk::ImageLayout ToVkImageLayout(ImageState state);
vk::PipelineStageFlags2 ToVkPipelineStage2(Stage stage);
vk::AccessFlags2 ToVkAccess2(Access access);
vk::PipelineStageFlags ToVkPipelineStage1(Stage stage);
vk::AccessFlags ToVkAccess1(Access access);

} // namespace UHE::RHI::VULKAN
