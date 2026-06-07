#include "uhepch.h"
#include "VulkanCommandBuffer.h"
#include <array>
#include <vulkan/vulkan_raii.hpp>
#include "Platform/Vulkan/VulkanBuffer.h"
#include "Platform/Vulkan/VulkanDescriptorManager.h"
#include "Platform/Vulkan/VulkanGraphicPipeline.h"
#include "Platform/Vulkan/VulkanTexture.h"
#include "UHE/RHI/RHITypes.h"
#include "UHE/Renderer/Shader.h"
#include "VulkanCommandPool.h"

#include "vulkan/vulkan.hpp"

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
    VulkanTexture* texture = reinterpret_cast<VulkanTexture*>(desc.colorAttachments[0].texture);
    m_CurrentRenderTarget = desc.colorAttachments[0].texture;

    vk::RenderingAttachmentInfo colorAttachment{};
    colorAttachment.imageView = *texture->GetImageView();
    colorAttachment.imageLayout = vk::ImageLayout::eColorAttachmentOptimal;
    colorAttachment.loadOp = vk::AttachmentLoadOp::eClear;
    colorAttachment.storeOp = vk::AttachmentStoreOp::eStore;
    colorAttachment.clearValue.color = vk::ClearColorValue(
        std::array<f32, 4>{desc.colorAttachments[0].clearColor.r, desc.colorAttachments[0].clearColor.g,
                           desc.colorAttachments[0].clearColor.b, desc.colorAttachments[0].clearColor.a});

    vk::RenderingInfo renderingInfo{};
    renderingInfo.renderArea.offset = vk::Offset2D{0, 0};
    renderingInfo.renderArea.extent = vk::Extent2D{desc.renderWidth, desc.renderHeight};
    renderingInfo.layerCount = 1;
    renderingInfo.colorAttachmentCount = 1;
    renderingInfo.pColorAttachments = &colorAttachment;

    vk::ImageMemoryBarrier barrier{};
    barrier.oldLayout = vk::ImageLayout::eUndefined;
    barrier.newLayout = vk::ImageLayout::eColorAttachmentOptimal;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = *texture->GetImage();
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
    VulkanTexture* texture = reinterpret_cast<VulkanTexture*>(m_CurrentRenderTarget);

    m_CommandBuffer.endRendering();

    vk::ImageMemoryBarrier barrier{};
    barrier.oldLayout = vk::ImageLayout::eColorAttachmentOptimal;
    barrier.newLayout = vk::ImageLayout::ePresentSrcKHR;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    if (texture)
    {
        barrier.image = *texture->GetImage();
    }
    barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.srcAccessMask = vk::AccessFlags{};

    m_CommandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eColorAttachmentOutput,
                                    vk::PipelineStageFlagBits::eBottomOfPipe, vk::DependencyFlags{}, nullptr, nullptr,
                                    barrier);
}

// ─── Pipeline & State Bindings ──────────────────────────────────

void VulkanCommandBuffer::BindPipeline(PipelineHandle handle)
{
    VulkanGraphicPipeline* pipeline = reinterpret_cast<VulkanGraphicPipeline*>(handle);

    m_CurrentPipelineLayout = pipeline->GetPipelineLayout();
    m_CommandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline->GetPipeline());

    if (m_DescriptorManager)
    {
        vk::DescriptorSet globalSet = m_DescriptorManager->GetSetHandle();
        m_CommandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_CurrentPipelineLayout, 0, {globalSet},
                                           nullptr);
    }
}

void VulkanCommandBuffer::BindVertexBuffer(BufferHandle handle, u64 offset)
{
    VulkanBuffer* buffer = reinterpret_cast<VulkanBuffer*>(handle);
    vk::Buffer vertexBuffers[] = {buffer->GetHandle()};
    vk::DeviceSize m_offset[] = {offset};
    m_CommandBuffer.bindVertexBuffers(0, vertexBuffers, m_offset);
}

void VulkanCommandBuffer::BindIndexBuffer(BufferHandle handle, u64 offset)
{
    VulkanBuffer* buffer = reinterpret_cast<VulkanBuffer*>(handle);
    m_CommandBuffer.bindIndexBuffer(buffer->GetHandle(), offset, vk::IndexType ::eUint32);
}

void VulkanCommandBuffer::BindTexture(u32 slot, TextureHandle handle)
{
    VulkanTexture* texture = reinterpret_cast<VulkanTexture*>(handle);
    if (texture && m_DescriptorManager && m_LogDevice)
    {
        m_DescriptorManager->BindTexture(*m_LogDevice, slot, *texture->GetImageView(), texture->GetSampler());
    }
}

// ─── Dynamic States ─────────────────────────────────────────────

void VulkanCommandBuffer::SetViewport(float x, float y, float width, float height)
{
    vk::Viewport viewPort{};
    viewPort.x = x;
    viewPort.y = y;
    viewPort.height = width;
    viewPort.width = height;
    viewPort.minDepth = 0.0f;
    viewPort.maxDepth = 1.0f;

    m_CommandBuffer.setViewport(1, viewPort);
}

void VulkanCommandBuffer::SetScissor(i32 x, i32 y, u32 width, u32 height)
{
    vk::Rect2D scissor{};
    scissor.offset = vk::Offset2D{x, y};
    scissor.extent = vk::Extent2D{width, height};
    m_CommandBuffer.setScissor(1, scissor);
}

// ─── Inline Data Paths ──────────────────────────────────────────

void VulkanCommandBuffer::PushConstants(ShaderStage stage, const void* data, u32 size, u32 offset)
{
    if (m_CurrentPipelineLayout)
    {
        vk::ShaderStageFlags flags{};
        if (stage == ShaderStage::Vertex)
            flags = vk::ShaderStageFlagBits::eVertex;
        else if (stage == ShaderStage::Fragment)
            flags = vk::ShaderStageFlagBits::eFragment;
        else if (stage == ShaderStage::Compute)
            flags = vk::ShaderStageFlagBits::eCompute;

        m_CommandBuffer.pushConstants(m_CurrentPipelineLayout, flags, offset, size, data);
    }
}

void VulkanCommandBuffer::UpdateBuffer(BufferHandle handle, const void* data, u64 size, u64 offset)
{
    VulkanBuffer* buffer = reinterpret_cast<VulkanBuffer*>(handle);
    buffer->UploadData(data, size);
}

void VulkanCommandBuffer::UpdateTexture(TextureHandle handle, const void* data, u64 size)
{
    VulkanTexture* Texture = reinterpret_cast<VulkanTexture*>(handle);
    Texture->UpdateTexture(data, size);
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
