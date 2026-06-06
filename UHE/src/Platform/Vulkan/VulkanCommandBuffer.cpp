#include "uhepch.h"
#include "VulkanCommandBuffer.h"
#include <array>
#include <vulkan/vulkan_raii.hpp>
#include "Platform/Vulkan/VulkanGraphicPipeline.h"
#include "Platform/Vulkan/VulkanTexture.h"
#include "UHE/RHI/RHITypes.h"
#include "VulkanCommandPool.h"

namespace UHE::RHI::VULKAN
{

// ─── Internal Vulkan-specific methods ───────────────────────────

void VulkanCommandBuffer::Allocate(vk::raii::Device& device, VulkanCommandPool& pool, bool isPrimary)
{
    vk::CommandBufferAllocateInfo allocInfo{};
    allocInfo.commandPool = *pool.GetHandle();
    allocInfo.level = isPrimary ? vk::CommandBufferLevel::ePrimary : vk::CommandBufferLevel::eSecondary;
    allocInfo.commandBufferCount = 1;

    vk::raii::CommandBuffers cmdBuffers(device, allocInfo);
    m_CommandBuffer = std::move(cmdBuffers[0]);
}

void VulkanCommandBuffer::Free()
{
    m_CommandBuffer.clear();
}

void VulkanCommandBuffer::BeginCommandBuffer(vk::CommandBufferUsageFlags flags)
{
    vk::CommandBufferBeginInfo beginInfo{};
    beginInfo.flags = flags;
    m_CommandBuffer.begin(beginInfo);
}

void VulkanCommandBuffer::Reset(vk::CommandBufferUsageFlags flags)
{
    vkResetCommandBuffer(*m_CommandBuffer, static_cast<VkCommandBufferResetFlags>(flags));
}

void VulkanCommandBuffer::EndCommandBuffer()
{
    m_CommandBuffer.end();
}

// ─── RHICommandBuffer overrides ─────────────────────────────────

void VulkanCommandBuffer::Begin()
{
    BeginCommandBuffer(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
}

void VulkanCommandBuffer::End()
{
    EndCommandBuffer();
}

// ─── Render Pass ────────────────────────────────────────────────

void VulkanCommandBuffer::BeginRenderPass(const RenderPassDesc& desc)
{

    TextureHandle handle = desc.colorAttachments[0].texture;
    VulkanTexture* texture = reinterpret_cast<VulkanTexture*>(handle);

    vk::ImageView ImageView = texture->GetImageView();

    vk::RenderingAttachmentInfo colorAttachment{};
    colorAttachment.imageView = m_swapChainClass.GetImageView(m_imageIndex);
    colorAttachment.imageLayout = vk::ImageLayout::eColorAttachmentOptimal;
    colorAttachment.loadOp = vk::AttachmentLoadOp::eClear;
    colorAttachment.storeOp = vk::AttachmentStoreOp::eStore;
    colorAttachment.clearValue.color = vk::ClearColorValue(
        std::array<f32, 4>{desc.colorAttachments[0].clearColor.r, desc.colorAttachments[0].clearColor.g,
                           desc.colorAttachments[0].clearColor.b, desc.colorAttachments[0].clearColor.a});

    vk::RenderingInfo renderingInfo{};
    renderingInfo.renderArea.offset = vk::Offset2D{0, 0};
    renderingInfo.renderArea.extent = m_swapChainClass.GetExtent();
    renderingInfo.layerCount = 1;
    renderingInfo.colorAttachmentCount = 1;
    renderingInfo.pColorAttachments = &colorAttachment;

    vk::ImageMemoryBarrier barrier{};
    barrier.oldLayout = vk::ImageLayout::eUndefined;
    barrier.newLayout = vk::ImageLayout::eColorAttachmentOptimal;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = m_swapChainClass.GetImages()[m_imageIndex];
    barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.srcAccessMask = vk::AccessFlags{};
    barrier.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;

    m_CommandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTopOfPipe,
                                    vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::DependencyFlags{}, nullptr,
                                    nullptr, barrier);

    m_CommandBuffer.beginRendering(renderingInfo);
}

void VulkanCommandBuffer::EndRenderPass()
{
    vk::ImageMemoryBarrier barrier{};
    barrier.oldLayout = vk::ImageLayout::eColorAttachmentOptimal;
    barrier.newLayout = vk::ImageLayout::ePresentSrcKHR;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = m_swapChainClass.GetImages()[m_imageIndex];
    barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
    barrier.subresourceRange.baseMipLevel = 1;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 1;
    barrier.srcAccessMask = vk::AccessFlags{};

    m_CommandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eColorAttachmentOutput,
                                    vk::PipelineStageFlagBits::eBottomOfPipe, vk::DependencyFlags{}, nullptr, nullptr,
                                    barrier);
}

// ─── Pipeline & State Bindings ──────────────────────────────────

void VulkanCommandBuffer::BindPipeline(PipelineHandle handle)
{
    VulkanGraphicPipeline* pipeline = reinterpret_cast<VulkanGraphicPipeline*>(handle);
    m_CurrentFrame = static_cast<u32>(pipeline);
    m_CommandBuffer.bindPipeline(vk::PipelineBindPoint, pipeline->GetPipeline());
}

void VulkanCommandBuffer::BindVertexBuffer(BufferHandle handle, u64 offset)
{
    // TODO: implement
}

void VulkanCommandBuffer::BindIndexBuffer(BufferHandle handle, u64 offset)
{
    // TODO: implement
}

void VulkanCommandBuffer::BindTexture(u32 slot, TextureHandle handle)
{
    // TODO: implement
}

// ─── Dynamic States ─────────────────────────────────────────────

void VulkanCommandBuffer::SetViewport(float x, float y, float width, float height)
{
    // TODO: implement
}

void VulkanCommandBuffer::SetScissor(i32 x, i32 y, u32 width, u32 height)
{
    // TODO: implement
}

// ─── Inline Data Paths ──────────────────────────────────────────

void VulkanCommandBuffer::PushConstants(ShaderStage stage, const void* data, u32 size, u32 offset)
{
    // TODO: implement
}

void VulkanCommandBuffer::UpdateBuffer(BufferHandle handle, const void* data, u64 size, u64 offset)
{
    // TODO: implement
}

void VulkanCommandBuffer::UpdateTexture(TextureHandle handle, const void* data, u64 size)
{
    // TODO: implement
}

// ─── Action Commands ────────────────────────────────────────────

void VulkanCommandBuffer::Draw(u32 vertexCount, u32 firstVertex)
{
    // TODO: implement
}

void VulkanCommandBuffer::DrawIndexed(u32 indexCount, u32 firstIndex, i32 vertexOffset)
{
    // TODO: implement
}

} // namespace UHE::RHI::VULKAN
