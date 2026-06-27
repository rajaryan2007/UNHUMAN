#include "uhepch.h"
#include "VulkanImGuiBackend.h"
#include <GLFW/glfw3.h>
// clang-format off
#include <imgui.h>
#include <ImGuizmo.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>
// clang-format on
#include "Platform/Vulkan/VulkanDevice.h"
#include "Platform/Vulkan/VulkanLogicalDevice.h"
#include "Platform/Vulkan/VulkanCommandBuffer.h"
#include "UHE/Core/Application.h"

namespace UHE::RHI::VULKAN
{
VulkanImGuiLayer::VulkanImGuiLayer(VulkanDevice* device) : ImGuiLayer(), m_Device(device) {}

void VulkanImGuiLayer::OnAttach()
{

    ImGuiLayer::OnAttach();

    GLFWwindow* window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
    ImGui_ImplGlfw_InitForVulkan(window, true);

    VkDescriptorPoolSize pool_sizes[] = {{VK_DESCRIPTOR_TYPE_SAMPLER, 1000},
                                         {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000},
                                         {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000},
                                         {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000},
                                         {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000},
                                         {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000},
                                         {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000},
                                         {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000},
                                         {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000},
                                         {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000},
                                         {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000}};

    vk::DescriptorPoolCreateInfo poolInfo{
        .flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
        .maxSets = static_cast<uint32_t>(1000 * IM_ARRAYSIZE(pool_sizes)),
        .poolSizeCount = static_cast<uint32_t>(IM_ARRAYSIZE(pool_sizes)),
        .pPoolSizes = reinterpret_cast<vk::DescriptorPoolSize*>(pool_sizes)
    };
    m_DescriptorPool = std::make_unique<vk::raii::DescriptorPool>(m_Device->getLogicalDevClass().getLogicalDevice(), poolInfo);

    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = *m_Device->getInstanceClass().getInstance();
    init_info.PhysicalDevice = *m_Device->getPhysicalDevClass().getPhysicalDevice();
    init_info.Device = *m_Device->getLogicalDevClass().getLogicalDevice();
    init_info.QueueFamily = m_Device->getPhysicalDevClass().getQueueFamilyIndices().graphicsFamily.value();
    init_info.Queue = *m_Device->GetGraphicsQueue();
    init_info.DescriptorPool = **m_DescriptorPool;
    init_info.MinImageCount = 3; // Typically 3 for triple buffering
    init_info.ImageCount = 3;
    init_info.PipelineInfoMain.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

    init_info.UseDynamicRendering = true;

    VkPipelineRenderingCreateInfoKHR pipelineRenderingCreateInfo = {};
    pipelineRenderingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR;
    static VkFormat colorFormat = static_cast<VkFormat>(m_Device->getSwapChainClass().GetSurfaceFormat().format); 
    pipelineRenderingCreateInfo.colorAttachmentCount = 1;
    pipelineRenderingCreateInfo.pColorAttachmentFormats = &colorFormat;

    init_info.PipelineInfoMain.PipelineRenderingCreateInfo = pipelineRenderingCreateInfo;

    ImGui_ImplVulkan_Init(&init_info);
}

void VulkanImGuiLayer::OnDetach()
{
    m_Device->WaitIdle();
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    m_DescriptorPool.reset();

    // Base class destroys ImGui Context
    ImGuiLayer::OnDetach();
}

void VulkanImGuiLayer::Begin()
{
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGuizmo::BeginFrame();
}

void VulkanImGuiLayer::End()
{
    ImGui::Render();

    ImDrawData* draw_data = ImGui::GetDrawData();

    auto* rhiCmd = static_cast<VulkanCommandBuffer*>(&m_Device->GetCurrentCommandBuffer());
    vk::raii::CommandBuffer& cmd = rhiCmd->GetHandle();

    auto& swapchain = m_Device->getSwapChainClass();
    u32 imageIndex = m_Device->ImageIndex();
    vk::Image swapchainImage = swapchain.GetImages()[imageIndex];
    vk::raii::ImageView& swapchainImageView = swapchain.GetImageView(imageIndex);
    vk::Extent2D extent = swapchain.GetExtent();

    
    vk::ImageMemoryBarrier barrier{
        .srcAccessMask = {},
        .dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eColorAttachmentRead,
        .oldLayout = vk::ImageLayout::eUndefined,
        .newLayout = vk::ImageLayout::eColorAttachmentOptimal,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = swapchainImage,
        .subresourceRange = {
            .aspectMask = vk::ImageAspectFlagBits::eColor,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1
        }
    };

    cmd.pipelineBarrier(vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::PipelineStageFlagBits::eColorAttachmentOutput, {}, nullptr, nullptr, barrier);

    
    vk::RenderingAttachmentInfo colorAttachment{
        .imageView = *swapchainImageView,
        .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
        .resolveMode = {},
        .resolveImageView = {},
        .resolveImageLayout = {},
        .loadOp = vk::AttachmentLoadOp::eClear,
        .storeOp = vk::AttachmentStoreOp::eStore,
        .clearValue = vk::ClearValue{vk::ClearColorValue{std::array<float, 4>{0.0f, 0.0f, 0.0f, 1.0f}}}
    };

    vk::RenderingInfo renderingInfo{
        .flags = {},
        .renderArea = { vk::Offset2D{0, 0}, extent },
        .layerCount = 1,
        .viewMask = 0,
        .colorAttachmentCount = 1,
        .pColorAttachments = &colorAttachment,
        .pDepthAttachment = nullptr,
        .pStencilAttachment = nullptr
    };

    cmd.beginRendering(renderingInfo);

    
    ImGui_ImplVulkan_RenderDrawData(draw_data, *cmd);

    
    cmd.endRendering();

    barrier.oldLayout = vk::ImageLayout::eColorAttachmentOptimal;
    barrier.newLayout = vk::ImageLayout::ePresentSrcKHR;
    barrier.srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
    barrier.dstAccessMask = {};

    cmd.pipelineBarrier(vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::PipelineStageFlagBits::eBottomOfPipe, {}, nullptr, nullptr, barrier);

    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
    }
}
} // namespace UHE::RHI::VULKAN
