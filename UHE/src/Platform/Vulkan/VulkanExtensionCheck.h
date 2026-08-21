#pragma once
#include <vector>
#include <vulkan/vulkan_raii.hpp>

namespace UHE::RHI::VULKAN
{
struct VulkanExtensionIsEnableCheck
{
    // ============= INSTANCE EXTENSIONS ================================================
    bool HasVkdebug_utils = false;         // VK_EXT_debug_utils
    bool HasVkAndroidSurface = false;      // VK_KHR_android_surface
    bool HasVkSwapchainColorspace = false; // VK_EXT_swapchain_colorspace

    // ======== DEVICE EXTENSIONS =================================================

    bool HasVkdynamicRendering = false;                    // VK_KHR_dynamic_rendering / Core 1.3
    bool HasVkSync2 = false;                               // VK_KHR_synchronization2 / Core 1.3
    bool HasVkTimelineSemaphore = false;                   // VK_KHR_timeline_semaphore / Core 1.2
    bool HasVkPushDescriptor = false;                      // VK_KHR_push_descriptor / Core 1.4
    bool HasVK16bit_storage = false;                       // VK_KHR_16bit_storage / Core 1.1
    bool HasVkshader_float16_int8 = false;                 // VK_KHR_shader_float16_int8 / Core 1.2
    bool HasVkbuffer_device_address = false;               // VK_KHR_buffer_device_address / Core 1.2
    bool HasVkshader_subgroup_extended_types = false;      // VK_KHR_shader_subgroup_extended_types / Core 1.2
    bool HasVkShaderObject = false;                        // VK_EXT_shader_object
    bool HasVkGraphicsPipelineLibrary = false;             // VK_EXT_graphics_pipeline_library
    bool HasVkBindlessDescriptor = false;                  // VK_EXT_descriptor_indexing / Core 1.2
    bool HasVkDescriptorBuffer = false;                    // VK_EXT_descriptor_buffer
    bool HasVkDescriptorHeap = false;                      // VK_EXT_descriptor_heap
    bool HasVkDrawIndirectCount = false;                   // VK_KHR_draw_indirect_count / Core 1.2
    bool HasVkInlineUniformBlock = false;                  // VK_EXT_inline_uniform_block / Core 1.3
    bool HasVkShaderDrawParameters = false;                // VK_KHR_shader_draw_parameters / Core 1.1
    bool HasVkmesh_shader = false;                         // VK_EXT_mesh_shader
    bool HasVkDeviceGeneratedCommands = false;             // VK_EXT_device_generated_commands
    bool HasVkExtendedDynamicState = false;                // VK_EXT_extended_dynamic_state / Core 1.3
    bool HasVkExtendedDynamicState2 = false;               // VK_EXT_extended_dynamic_state2 / Core 1.3
    bool HasVkExtendedDynamicState3 = false;               // VK_EXT_extended_dynamic_state3
    bool HasVkVideoQueue = false;                          // VK_KHR_video_queue
    bool HasVkdecode_av1 = false;                          // VK_KHR_video_decode_av1
    bool HasVkdecode_h265 = false;                         // VK_KHR_video_decode_h265
    bool HasVkdecode_h264 = false;                         // VK_KHR_video_decode_h264
    bool HasVkencode_h265 = false;                         // VK_KHR_video_encode_h265
    bool HasVkencode_h264 = false;                         // VK_KHR_video_encode_h264
    bool HasVkencode_av1 = false;                          // VK_KHR_video_encode_av1
    bool HasVKYcbcr2Conversion = false;                    // VK_KHR_sampler_ycbcr_conversion / Core 1.1
    bool HasVkVideoEncodeAV1 = false;                      // VK_KHR_video_encode_av1
    bool HasVkVideoEncodeFeedback2 = false;                // VK_KHR_video_encode_feedback2
    bool HasVkShaderFramebufferFetch = false;              // VK_EXT_shader_framebuffer_fetch
    bool HasVkMemoryBudget = false;                        // VK_EXT_memory_budget
    bool HasVkTextureCompressionASTC_HDR = false;          // VK_EXT_texture_compression_astc_hdr
    bool HasVkExternalMemoryAndroidHardwareBuffer = false; // VK_ANDROID_external_memory_android_hardware_buffer
    bool HasVkMultiview = false;                           // VK_KHR_multiview / Core 1.1
    bool HasVkMaintenance1 = false;                        // VK_KHR_maintenance1 / Core 1.1
    bool HasVkMaintenance2 = false;                        // VK_KHR_maintenance2 / Core 1.1
    bool HasVkMaintenance3 = false;                        // VK_KHR_maintenance3 / Core 1.1
    bool HasVkDedicatedAllocation = false;                 // VK_KHR_dedicated_allocation / Core 1.1
    bool HasVkCreateRenderPass2 = false;                   // VK_KHR_create_renderpass2 / Core 1.2
    bool HasVkMemoryPriority = false;                      // VK_EXT_memory_priority
    bool HasVkPageableDeviceLocalMemory = false;           // VK_EXT_pageable_device_local_memory
    bool HasVkHostImageCopy = false;                       // VK_EXT_host_image_copy / Core 1.4
    bool HasVkDynamicRenderingLocalRead = false;           // VK_KHR_dynamic_rendering_local_read / Core 1.4
    bool HasVkAccelerationStructure = false;               // VK_KHR_acceleration_structure
    bool HasVkDeferredHostOperations = false;              // VK_KHR_deferred_host_operations
    bool HasVkRayTracingPipeline = false;                  // VK_KHR_ray_tracing_pipeline
    bool HasVkRayQuery = false;                            // VK_KHR_ray_query
    bool HasVkfragment_shading_rate = false;               // VK_KHR_fragment_shading_rate
    bool HasVkCooperativeMatrix = false;                   // VK_KHR_cooperative_matrix
    bool HasVkRobustness2 = false;                         // VK_EXT_robustness2
    bool HasVkImageRobustness = false;                     // VK_EXT_image_robustness / Core 1.3
    bool HasVkperformance_query = false;                   // VK_KHR_performance_query
    bool HasVkcalibrated_timestamps = false;               // VK_KHR_calibrated_timestamps / Core 1.4
    bool HasVkHostQueryReset = false;                      // VK_EXT_host_query_reset / Core 1.2
};
class VulkanExtensionCheck
{
public:
    VulkanExtensionCheck() = default;
    ~VulkanExtensionCheck() = default;

    [[nodiscard]] const VulkanExtensionIsEnableCheck GetVulkanExtensionFlags() const noexcept
    {
        return m_extensionCheck;
    };
    [[nodiscard("extensions check don't get used")]] bool IsEnable(const std::string_view ExtenstionName) const noexcept
    {
        auto extensions = GetEnabledDeviceExtensions();
        for (const auto* ext : extensions)
        {
            if (ExtenstionName == ext)
            {
                return true;
            }
        }
        return false;
    };
    [[nodiscard]] std::vector<const char*> GetEnabledDeviceExtensions() const;
    [[nodiscard]] vk::PhysicalDeviceFeatures2* BuildDeviceFeatureChain();
    void TickTheAvailableExtension(const vk::raii::PhysicalDevice& PhysicalDevice);

private:
    VulkanExtensionIsEnableCheck m_extensionCheck;
    vk::PhysicalDeviceFeatures2 m_features2;
    vk::PhysicalDeviceVulkan11Features m_v11Features;
    vk::PhysicalDeviceVulkan12Features m_v12Features;
    vk::PhysicalDeviceVulkan13Features m_v13Features;
    vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT m_dynamicStateFeatures;
    vk::PhysicalDeviceExtendedDynamicState2FeaturesEXT m_dynamicState2Features;
    vk::PhysicalDeviceDescriptorBufferFeaturesEXT m_descriptorBufferFeatures;
    vk::PhysicalDeviceShaderObjectFeaturesEXT m_shaderObjectFeatures;
    vk::PhysicalDeviceGraphicsPipelineLibraryFeaturesEXT m_graphicsPipelineLibraryFeatures;
    vk::PhysicalDeviceMeshShaderFeaturesEXT m_meshShaderFeatures;
    vk::PhysicalDeviceAccelerationStructureFeaturesKHR m_accelerationStructureFeatures;
    vk::PhysicalDeviceRayTracingPipelineFeaturesKHR m_rayTracingPipelineFeatures;
    vk::PhysicalDeviceRayQueryFeaturesKHR m_rayQueryFeatures;
    vk::PhysicalDeviceFragmentShadingRateFeaturesKHR m_fragmentShadingRateFeatures;
    vk::PhysicalDeviceCooperativeMatrixFeaturesKHR m_cooperativeMatrixFeatures;
    vk::PhysicalDeviceRobustness2FeaturesEXT m_robustness2Features;
    vk::PhysicalDeviceMemoryPriorityFeaturesEXT m_memoryPriorityFeatures;
    vk::PhysicalDevicePageableDeviceLocalMemoryFeaturesEXT m_pageableDeviceLocalMemoryFeatures;
    vk::PhysicalDeviceHostImageCopyFeaturesEXT m_hostImageCopyFeatures;
    vk::PhysicalDeviceDynamicRenderingLocalReadFeaturesKHR m_dynamicRenderingLocalReadFeatures;
};

} // namespace UHE::RHI::VULKAN
