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
#include "UHE/Scene/Scene.h"
#include "VulkanCommandPool.h"
#include "vulkan/vulkan.hpp"
#include "UHE/Renderer/Renderer.h"
#include "Platform/Vulkan/VulkanDevice.h"

namespace UHE::RHI::VULKAN
{

// ─── Internal Vulkan-specific methods ───────────────────────────

void VulkanCommandBuffer::Allocate(const vk::raii::Device& device, VulkanCommandPool& pool, bool isPrimary)
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
    m_CommandBuffer.reset(vk::CommandBufferResetFlags(static_cast<VkCommandBufferResetFlags>(flags)));
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
    auto* texture = reinterpret_cast<VulkanTexture*>(desc.colorAttachments[0].texture);
    m_CurrentRenderTarget = desc.colorAttachments[0].texture;

    vk::Image image;
    vk::ImageView imageView;

    if (texture) {
        image = *texture->GetImage();
        imageView = *texture->GetImageView();
    } else {
        auto& device = UHE::Renderer::GetDevice();
        auto& vulkanDevice = static_cast<VulkanDevice&>(device);
        auto& swapChain = vulkanDevice.getSwapChainClass();
        u32 index = vulkanDevice.ImageIndex();
        image = swapChain.GetImages()[index];
        imageView = *swapChain.GetImageView(index);
    }

    vk::RenderingAttachmentInfo colorAttachment{};
    colorAttachment.imageView = imageView;
    colorAttachment.imageLayout = vk::ImageLayout::eColorAttachmentOptimal;
    colorAttachment.loadOp = vk::AttachmentLoadOp::eClear;
    colorAttachment.storeOp = vk::AttachmentStoreOp::eStore;
    colorAttachment.clearValue.color = vk::ClearColorValue(
        std::array<f32, 4>{desc.colorAttachments[0].clearColor.r, desc.colorAttachments[0].clearColor.g,
                           desc.colorAttachments[0].clearColor.b, desc.colorAttachments[0].clearColor.a});

    vk::Extent2D renderExtent;
    if (texture) {
        renderExtent = vk::Extent2D{desc.renderWidth, desc.renderHeight};
    } else {
        auto& device = UHE::Renderer::GetDevice();
        auto& vulkanDevice = static_cast<VulkanDevice&>(device);
        renderExtent = vulkanDevice.getSwapChainClass().GetExtent();
    }

    vk::RenderingInfo renderingInfo{};
    renderingInfo.renderArea.offset = vk::Offset2D{0, 0};
    renderingInfo.renderArea.extent = renderExtent;
    renderingInfo.layerCount = 1;
    renderingInfo.colorAttachmentCount = 1;
    renderingInfo.pColorAttachments = &colorAttachment;

    vk::ImageMemoryBarrier barrier{};
    barrier.oldLayout = vk::ImageLayout::eUndefined;
    barrier.newLayout = vk::ImageLayout::eColorAttachmentOptimal;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
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
    auto* texture = reinterpret_cast<VulkanTexture*>(m_CurrentRenderTarget);

    m_CommandBuffer.endRendering();

    vk::ImageMemoryBarrier barrier{};
    barrier.oldLayout = vk::ImageLayout::eColorAttachmentOptimal;
    barrier.newLayout = vk::ImageLayout::ePresentSrcKHR;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    vk::Image image;
    if (texture)
    {
        image = *texture->GetImage();
    }
    else
    {
        auto& device = UHE::Renderer::GetDevice();
        auto& vulkanDevice = static_cast<VulkanDevice&>(device);
        auto& swapChain = vulkanDevice.getSwapChainClass();
        u32 index = vulkanDevice.ImageIndex();
        image = swapChain.GetImages()[index];
    }
    barrier.image = image;
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
    auto* buffer = reinterpret_cast<VulkanBuffer*>(handle);
    std::array<vk::Buffer, 1> vertexBuffers = {buffer->GetHandle()};
    std::array<vk::DeviceSize, 1> m_offset = {offset};
    m_CommandBuffer.bindVertexBuffers(0, vertexBuffers, m_offset);
}

void VulkanCommandBuffer::BindIndexBuffer(BufferHandle handle, u64 offset)
{
    auto* buffer = reinterpret_cast<VulkanBuffer*>(handle);
    m_CommandBuffer.bindIndexBuffer(buffer->GetHandle(), offset, vk::IndexType ::eUint32);
}

void VulkanCommandBuffer::BindTexture(u32 slot, TextureHandle handle)
{
    auto* texture = reinterpret_cast<VulkanTexture*>(handle);
    if (texture != nullptr && m_DescriptorManager != nullptr && m_LogDevice != nullptr)
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
    viewPort.width = width;
    viewPort.height = height;
    viewPort.minDepth = 0.0f;
    viewPort.maxDepth = 1.0f;
    m_CommandBuffer.setViewport(0, viewPort);
}

void VulkanCommandBuffer::SetScissor(i32 x, i32 y, u32 width, u32 height)
{
    vk::Rect2D scissor{};
    scissor.offset = vk::Offset2D{x, y};
    scissor.extent = vk::Extent2D{width, height};
    m_CommandBuffer.setScissor(0, scissor);
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
    auto* buffer = reinterpret_cast<VulkanBuffer*>(handle);
    buffer->UploadData(data, size);
}

void VulkanCommandBuffer::UpdateTexture(TextureHandle handle, const void* data, u64 size)
{
    auto* Texture = reinterpret_cast<VulkanTexture*>(handle);
    Texture->UpdateTexture(data, size);
}

// ─── Action Commands ────────────────────────────────────────────

void VulkanCommandBuffer::Draw(u32 vertexCount, u32 firstVertex)
{
    m_CommandBuffer.draw(vertexCount, 1, firstVertex, 0);
}

void VulkanCommandBuffer::DrawIndexed(u32 indexCount, u32 firstIndex, i32 vertexOffset)
{
    m_CommandBuffer.drawIndexed(indexCount, 1, firstIndex, vertexOffset, 0);
}

} // namespace UHE::RHI::VULKAN
