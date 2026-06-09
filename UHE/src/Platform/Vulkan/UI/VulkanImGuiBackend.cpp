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

    VkDescriptorPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    pool_info.maxSets = 1000 * IM_ARRAYSIZE(pool_sizes);
    pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
    pool_info.pPoolSizes = pool_sizes;

    vkCreateDescriptorPool(*m_Device->getLogicalDevClass().getLogicalDevice(), &pool_info, nullptr, &m_DescriptorPool);

    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = *m_Device->getInstanceClass().getInstance();
    init_info.PhysicalDevice = *m_Device->getPhysicalDevClass().getPhysicalDevice();
    init_info.Device = *m_Device->getLogicalDevClass().getLogicalDevice();
    init_info.QueueFamily = m_Device->getPhysicalDevClass().getQueueFamilyIndices().graphicsFamily.value();
    init_info.Queue = *m_Device->GetGraphicsQueue();
    init_info.DescriptorPool = m_DescriptorPool;
    init_info.MinImageCount = 3; // Typically 3 for triple buffering
    init_info.ImageCount = 3;
    init_info.PipelineInfoMain.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

    init_info.UseDynamicRendering = true;

    VkPipelineRenderingCreateInfoKHR pipelineRenderingCreateInfo = {};
    pipelineRenderingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR;
    static VkFormat colorFormat = VK_FORMAT_B8G8R8A8_UNORM; // Static so pointer remains valid
    pipelineRenderingCreateInfo.colorAttachmentCount = 1;
    pipelineRenderingCreateInfo.pColorAttachmentFormats = &colorFormat;

    init_info.PipelineInfoMain.PipelineRenderingCreateInfo = pipelineRenderingCreateInfo;

    ImGui_ImplVulkan_Init(&init_info);
}

void VulkanImGuiLayer::OnDetach()
{
    vkDeviceWaitIdle(*m_Device->getLogicalDevClass().getLogicalDevice());
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    vkDestroyDescriptorPool(*m_Device->getLogicalDevClass().getLogicalDevice(), m_DescriptorPool, nullptr);

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

    // NOTE: The actual drawing ( ImGui_ImplVulkan_RenderDrawData ) will happen
    // inside your main render loop when you record your command buffers!

    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
    }
}
} // namespace UHE::RHI::VULKAN
