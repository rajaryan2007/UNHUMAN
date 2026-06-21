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
    std::vector<vk::RenderingAttachmentInfo> colorAttachments;
    std::vector<vk::ImageMemoryBarrier> barriers;

    vk::Extent2D renderExtent;
    m_CurrentRenderPassDesc = desc;

    if (desc.colorAttachmentCount == 0) {
        // Swapchain fallback
        auto& device = UHE::Renderer::GetDevice();
        auto& vulkanDevice = static_cast<VulkanDevice&>(device);
        auto& swapChain = vulkanDevice.getSwapChainClass();
        u32 index = vulkanDevice.ImageIndex();
        vk::Image image = swapChain.GetImages()[index];
        vk::ImageView imageView = *swapChain.GetImageView(index);

        vk::RenderingAttachmentInfo colorAttachment{};
        colorAttachment.imageView = imageView;
        colorAttachment.imageLayout = vk::ImageLayout::eColorAttachmentOptimal;
        colorAttachment.loadOp = vk::AttachmentLoadOp::eClear;
        colorAttachment.storeOp = vk::AttachmentStoreOp::eStore;
        colorAttachment.clearValue.color = vk::ClearColorValue(std::array<f32, 4>{0.0f, 0.0f, 0.0f, 1.0f});

        colorAttachments.push_back(colorAttachment);
        renderExtent = swapChain.GetExtent();

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
        barriers.push_back(barrier);
    } else {
        renderExtent = vk::Extent2D{desc.renderWidth, desc.renderHeight};

        for (u32 i = 0; i < desc.colorAttachmentCount; i++) {
            auto* texture = reinterpret_cast<VulkanTexture*>(desc.colorAttachments[i].texture);
            if (!texture) continue;
            
            vk::Image image = texture->GetImage();
            vk::ImageView imageView = *texture->GetImageView();

            vk::RenderingAttachmentInfo colorAttachment{};
            colorAttachment.imageView = imageView;
            colorAttachment.imageLayout = vk::ImageLayout::eColorAttachmentOptimal;
            colorAttachment.loadOp = vk::AttachmentLoadOp::eClear;
            colorAttachment.storeOp = vk::AttachmentStoreOp::eStore;
            
            if (texture->GetDesc().format == RHI::TextureFormat::R32_SINT) {
                colorAttachment.clearValue.color = vk::ClearColorValue(
                    std::array<int32_t, 4>{(int32_t)desc.colorAttachments[i].clearColor.r, 0, 0, 0});
            } else {
                colorAttachment.clearValue.color = vk::ClearColorValue(
                    std::array<f32, 4>{desc.colorAttachments[i].clearColor.r, desc.colorAttachments[i].clearColor.g,
                                       desc.colorAttachments[i].clearColor.b, desc.colorAttachments[i].clearColor.a});
            }
            
            colorAttachments.push_back(colorAttachment);

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
            barriers.push_back(barrier);
        }
    }

    vk::RenderingAttachmentInfo depthAttachmentInfo{};
    if (desc.hasDepth && desc.depthAttachment.texture) {
        auto* depthTex = reinterpret_cast<VulkanTexture*>(desc.depthAttachment.texture);
        vk::Image image = depthTex->GetImage();
        
        depthAttachmentInfo.imageView = *depthTex->GetImageView();
        depthAttachmentInfo.imageLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
        depthAttachmentInfo.loadOp = vk::AttachmentLoadOp::eClear;
        depthAttachmentInfo.storeOp = vk::AttachmentStoreOp::eStore;
        depthAttachmentInfo.clearValue.depthStencil = vk::ClearDepthStencilValue(1.0f, 0);

        vk::ImageMemoryBarrier barrier{};
        barrier.oldLayout = vk::ImageLayout::eUndefined;
        barrier.newLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = image;
        barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;
        barrier.srcAccessMask = vk::AccessFlags{};
        barrier.dstAccessMask = vk::AccessFlagBits::eDepthStencilAttachmentWrite;
        barriers.push_back(barrier);
    }

    if (!barriers.empty()) {
        m_CommandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eColorAttachmentOutput,
                                        vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests,
                                        vk::DependencyFlags{}, nullptr, nullptr, barriers);
    }

    vk::RenderingInfo renderingInfo{};
    renderingInfo.renderArea.offset = vk::Offset2D{0, 0};
    renderingInfo.renderArea.extent = renderExtent;
    renderingInfo.layerCount = 1;
    renderingInfo.colorAttachmentCount = static_cast<u32>(colorAttachments.size());
    renderingInfo.pColorAttachments = colorAttachments.empty() ? nullptr : colorAttachments.data();
    if (desc.hasDepth && desc.depthAttachment.texture) {
        renderingInfo.pDepthAttachment = &depthAttachmentInfo;
        renderingInfo.pStencilAttachment = &depthAttachmentInfo;
    }

    m_CommandBuffer.beginRendering(renderingInfo);
}

void VulkanCommandBuffer::EndRenderPass()
{
    m_CommandBuffer.endRendering();

    std::vector<vk::ImageMemoryBarrier> barriers;

    if (m_CurrentRenderPassDesc.colorAttachmentCount == 0) {
        auto& device = UHE::Renderer::GetDevice();
        auto& vulkanDevice = static_cast<VulkanDevice&>(device);
        auto& swapChain = vulkanDevice.getSwapChainClass();
        u32 index = vulkanDevice.ImageIndex();
        vk::Image image = swapChain.GetImages()[index];

        vk::ImageMemoryBarrier barrier{};
        barrier.oldLayout = vk::ImageLayout::eColorAttachmentOptimal;
        barrier.newLayout = vk::ImageLayout::ePresentSrcKHR;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = image;
        barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;
        barrier.srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
        barrier.dstAccessMask = vk::AccessFlags{};
        barriers.push_back(barrier);
    } else {
        for (u32 i = 0; i < m_CurrentRenderPassDesc.colorAttachmentCount; i++) {
            auto* texture = reinterpret_cast<VulkanTexture*>(m_CurrentRenderPassDesc.colorAttachments[i].texture);
            if (!texture) continue;

            vk::Image image = texture->GetImage();

            vk::ImageMemoryBarrier barrier{};
            barrier.oldLayout = vk::ImageLayout::eColorAttachmentOptimal;
            barrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal; // To read in ImGui
            barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.image = image;
            barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
            barrier.subresourceRange.baseMipLevel = 0;
            barrier.subresourceRange.levelCount = 1;
            barrier.subresourceRange.baseArrayLayer = 0;
            barrier.subresourceRange.layerCount = 1;
            barrier.srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
            barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
            barriers.push_back(barrier);
        }

        if (m_CurrentRenderPassDesc.hasDepth && m_CurrentRenderPassDesc.depthAttachment.texture) {
            auto* depthTex = reinterpret_cast<VulkanTexture*>(m_CurrentRenderPassDesc.depthAttachment.texture);
            vk::Image image = depthTex->GetImage();

            vk::ImageMemoryBarrier barrier{};
            barrier.oldLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
            barrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
            barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.image = image;
            barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil;
            barrier.subresourceRange.baseMipLevel = 0;
            barrier.subresourceRange.levelCount = 1;
            barrier.subresourceRange.baseArrayLayer = 0;
            barrier.subresourceRange.layerCount = 1;
            barrier.srcAccessMask = vk::AccessFlagBits::eDepthStencilAttachmentWrite;
            barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
            barriers.push_back(barrier);
        }
    }

    if (!barriers.empty()) {
        m_CommandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eLateFragmentTests,
                                        vk::PipelineStageFlagBits::eBottomOfPipe | vk::PipelineStageFlagBits::eFragmentShader,
                                        vk::DependencyFlags{}, nullptr, nullptr, barriers);
    }
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
    // No-op for global bindless textures
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
        else if (stage == ShaderStage::AllGraphics)
            flags = vk::ShaderStageFlagBits::eAllGraphics;

        m_CommandBuffer.pushConstants<uint8_t>(m_CurrentPipelineLayout, static_cast<vk::ShaderStageFlags>(flags), offset, vk::ArrayProxy<const uint8_t>(size, static_cast<const uint8_t*>(data)));
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
