#include "uhepch.h"
#include "VulkanExtensionCheck.h"
#include <vector>
#include <volk.h>
#include <vulkan/vulkan_core.h>
#include "vulkan/vulkan.hpp"

namespace UHE::RHI::VULKAN
{

std::vector<const char*> VulkanExtensionCheck::GetEnabledDeviceExtensions() const
{
    std::vector<const char*> extensions;

    extensions.emplace_back("VK_KHR_swapchain");

    if (m_extensionCheck.HasVkdynamicRendering)
        extensions.emplace_back("VK_KHR_dynamic_rendering");
    if (m_extensionCheck.HasVkSync2)
        extensions.emplace_back("VK_KHR_synchronization2");
    if (m_extensionCheck.HasVkTimelineSemaphore)
        extensions.emplace_back("VK_KHR_timeline_semaphore");
    if (m_extensionCheck.HasVkPushDescriptor)
        extensions.emplace_back("VK_KHR_push_descriptor");

    if (m_extensionCheck.HasVK16bit_storage)
        extensions.emplace_back("VK_KHR_16bit_storage");
    if (m_extensionCheck.HasVkshader_float16_int8)
        extensions.emplace_back("VK_KHR_shader_float16_int8");
    if (m_extensionCheck.HasVkbuffer_device_address)
        extensions.emplace_back("VK_KHR_buffer_device_address");
    if (m_extensionCheck.HasVkshader_subgroup_extended_types)
        extensions.emplace_back("VK_KHR_shader_subgroup_extended_types");

    if (m_extensionCheck.HasVkBindlessDescriptor)
        extensions.emplace_back("VK_EXT_descriptor_indexing");
    if (m_extensionCheck.HasVkDescriptorBuffer)
        extensions.emplace_back("VK_EXT_descriptor_buffer");
    if (m_extensionCheck.HasVkDescriptorHeap)
        extensions.emplace_back("VK_EXT_descriptor_heap");

    if (m_extensionCheck.HasVkShaderObject)
        extensions.emplace_back("VK_EXT_shader_object");
    if (m_extensionCheck.HasVkGraphicsPipelineLibrary)
    {
        extensions.emplace_back("VK_EXT_graphics_pipeline_library");
        extensions.emplace_back("VK_KHR_pipeline_library");
    }
    if (m_extensionCheck.HasVkExtendedDynamicState)
        extensions.emplace_back("VK_EXT_extended_dynamic_state");
    if (m_extensionCheck.HasVkExtendedDynamicState2)
        extensions.emplace_back("VK_EXT_extended_dynamic_state2");
    if (m_extensionCheck.HasVkExtendedDynamicState3)
        extensions.emplace_back("VK_EXT_extended_dynamic_state3");

    if (m_extensionCheck.HasVkDrawIndirectCount)
        extensions.emplace_back("VK_KHR_draw_indirect_count");
    if (m_extensionCheck.HasVkInlineUniformBlock)
        extensions.emplace_back("VK_EXT_inline_uniform_block");
    if (m_extensionCheck.HasVkShaderDrawParameters)
        extensions.emplace_back("VK_KHR_shader_draw_parameters");
    if (m_extensionCheck.HasVkmesh_shader)
        extensions.emplace_back("VK_EXT_mesh_shader");
    if (m_extensionCheck.HasVkDeviceGeneratedCommands)
    {
        extensions.emplace_back("VK_EXT_device_generated_commands");
        extensions.emplace_back("VK_KHR_maintenance5");
    }

    if (m_extensionCheck.HasVkVideoQueue)
        extensions.emplace_back("VK_KHR_video_queue");
    if (m_extensionCheck.HasVkdecode_av1)
        extensions.emplace_back("VK_KHR_video_decode_av1");
    if (m_extensionCheck.HasVkdecode_h265)
        extensions.emplace_back("VK_KHR_video_decode_h265");
    if (m_extensionCheck.HasVkdecode_h264)
        extensions.emplace_back("VK_KHR_video_decode_h264");
    if (m_extensionCheck.HasVkencode_h265)
        extensions.emplace_back("VK_KHR_video_encode_h265");
    if (m_extensionCheck.HasVkencode_h264)
        extensions.emplace_back("VK_KHR_video_encode_h264");
    if (m_extensionCheck.HasVkencode_av1)
        extensions.emplace_back("VK_KHR_video_encode_av1");
    if (m_extensionCheck.HasVkVideoEncodeAV1)
        extensions.emplace_back("VK_KHR_video_encode_av1");
    if (m_extensionCheck.HasVkdecode_av1 || m_extensionCheck.HasVkdecode_h265 || m_extensionCheck.HasVkdecode_h264)
        extensions.emplace_back("VK_KHR_video_decode_queue");
    if (m_extensionCheck.HasVkencode_h265 || m_extensionCheck.HasVkencode_h264 || m_extensionCheck.HasVkencode_av1 ||
        m_extensionCheck.HasVkVideoEncodeAV1)
        extensions.emplace_back("VK_KHR_video_encode_queue");

    if (m_extensionCheck.HasVkVideoEncodeFeedback2)
        extensions.emplace_back("VK_KHR_video_encode_feedback2");
    if (m_extensionCheck.HasVKYcbcr2Conversion)
        extensions.emplace_back("VK_KHR_sampler_ycbcr_conversion");

    if (m_extensionCheck.HasVkExternalMemoryAndroidHardwareBuffer)
        extensions.emplace_back("VK_ANDROID_external_memory_android_hardware_buffer");
    if (m_extensionCheck.HasVkShaderFramebufferFetch)
        extensions.emplace_back("VK_EXT_shader_framebuffer_fetch");
    if (m_extensionCheck.HasVkMemoryBudget)
        extensions.emplace_back("VK_EXT_memory_budget");
    if (m_extensionCheck.HasVkTextureCompressionASTC_HDR)
        extensions.emplace_back("VK_EXT_texture_compression_astc_hdr");

    if (m_extensionCheck.HasVkMemoryPriority)
        extensions.emplace_back("VK_EXT_memory_priority");
    if (m_extensionCheck.HasVkPageableDeviceLocalMemory)
        extensions.emplace_back("VK_EXT_pageable_device_local_memory");
    if (m_extensionCheck.HasVkHostImageCopy)
        extensions.emplace_back("VK_EXT_host_image_copy");
    if (m_extensionCheck.HasVkDynamicRenderingLocalRead)
        extensions.emplace_back("VK_KHR_dynamic_rendering_local_read");
    if (m_extensionCheck.HasVkDedicatedAllocation)
        extensions.emplace_back("VK_KHR_dedicated_allocation");
    if (m_extensionCheck.HasVkAccelerationStructure)
    {
        extensions.emplace_back("VK_KHR_acceleration_structure");
        if (m_extensionCheck.HasVkDeferredHostOperations)
            extensions.emplace_back("VK_KHR_deferred_host_operations");
    }
    if (m_extensionCheck.HasVkRayTracingPipeline)
        extensions.emplace_back("VK_KHR_ray_tracing_pipeline");
    if (m_extensionCheck.HasVkRayQuery)
        extensions.emplace_back("VK_KHR_ray_query");
    if (m_extensionCheck.HasVkfragment_shading_rate)
        extensions.emplace_back("VK_KHR_fragment_shading_rate");
    if (m_extensionCheck.HasVkCooperativeMatrix)
        extensions.emplace_back("VK_KHR_cooperative_matrix");

    if (m_extensionCheck.HasVkRobustness2)
        extensions.emplace_back("VK_EXT_robustness2");
    if (m_extensionCheck.HasVkImageRobustness)
        extensions.emplace_back("VK_EXT_image_robustness");
    if (m_extensionCheck.HasVkCreateRenderPass2)
        extensions.emplace_back("VK_KHR_create_renderpass2");

    return extensions;
};
vk::PhysicalDeviceFeatures2* VulkanExtensionCheck::BuildDeviceFeatureChain()
{
    // ── Core Physical Device Features ──
    m_features2.features.samplerAnisotropy = VK_TRUE;
    m_features2.features.independentBlend = VK_TRUE;

    // ── Always chain Vulkan 1.1 / 1.2 / 1.3 (they are core for Vulkan 1.3) ──
    m_features2.pNext = &m_v11Features;
    m_v11Features.pNext = &m_v12Features;
    m_v12Features.pNext = &m_v13Features;

    // ── Vulkan 1.1 Features ──
    if (m_extensionCheck.HasVkShaderDrawParameters)
        m_v11Features.shaderDrawParameters = VK_TRUE;
    if (m_extensionCheck.HasVK16bit_storage)
        m_v11Features.storageBuffer16BitAccess = VK_TRUE;
    if (m_extensionCheck.HasVkMultiview)
        m_v11Features.multiview = VK_TRUE;
    if (m_extensionCheck.HasVKYcbcr2Conversion)
        m_v11Features.samplerYcbcrConversion = VK_TRUE;

    // ── Vulkan 1.2 Features ──
    if (m_extensionCheck.HasVkBindlessDescriptor)
    {
        m_v12Features.descriptorIndexing = VK_TRUE;
        m_v12Features.descriptorBindingStorageBufferUpdateAfterBind = VK_TRUE;
        m_v12Features.descriptorBindingPartiallyBound = VK_TRUE;
        m_v12Features.descriptorBindingSampledImageUpdateAfterBind = VK_TRUE;
        m_v12Features.shaderSampledImageArrayNonUniformIndexing = VK_TRUE;
        m_v12Features.runtimeDescriptorArray = VK_TRUE;
    }
    if (m_extensionCheck.HasVkbuffer_device_address)
        m_v12Features.bufferDeviceAddress = VK_TRUE;
    if (m_extensionCheck.HasVkTimelineSemaphore)
        m_v12Features.timelineSemaphore = VK_TRUE;
    if (m_extensionCheck.HasVkshader_float16_int8)
    {
        m_v12Features.shaderFloat16 = VK_TRUE;
        m_v12Features.shaderInt8 = VK_TRUE;
    }
    if (m_extensionCheck.HasVkshader_subgroup_extended_types)
        m_v12Features.shaderSubgroupExtendedTypes = VK_TRUE;
    if (m_extensionCheck.HasVkDrawIndirectCount)
        m_v12Features.drawIndirectCount = VK_TRUE;
    if (m_extensionCheck.HasVkHostQueryReset)
        m_v12Features.hostQueryReset = VK_TRUE;

    // ── Vulkan 1.3 Features ──
    if (m_extensionCheck.HasVkdynamicRendering)
        m_v13Features.dynamicRendering = VK_TRUE;
    if (m_extensionCheck.HasVkSync2)
        m_v13Features.synchronization2 = VK_TRUE;
    if (m_extensionCheck.HasVkInlineUniformBlock)
        m_v13Features.inlineUniformBlock = VK_TRUE;
    if (m_extensionCheck.HasVkImageRobustness)
        m_v13Features.robustImageAccess = VK_TRUE;
    if (m_extensionCheck.HasVkTextureCompressionASTC_HDR)
        m_v13Features.textureCompressionASTC_HDR = VK_TRUE;

    // ── Extension Feature Structs (appended to the pNext chain after v13) ──
    void** pNextChainTail = &m_v13Features.pNext;

    if (m_extensionCheck.HasVkExtendedDynamicState)
    {
        m_dynamicStateFeatures.extendedDynamicState = VK_TRUE;
        *pNextChainTail = &m_dynamicStateFeatures;
        pNextChainTail = &m_dynamicStateFeatures.pNext;
    }
    if (m_extensionCheck.HasVkExtendedDynamicState2)
    {
        m_dynamicState2Features.extendedDynamicState2 = VK_TRUE;
        *pNextChainTail = &m_dynamicState2Features;
        pNextChainTail = &m_dynamicState2Features.pNext;
    }
    if (m_extensionCheck.HasVkDescriptorBuffer)
    {
        m_descriptorBufferFeatures.descriptorBuffer = VK_TRUE;
        *pNextChainTail = &m_descriptorBufferFeatures;
        pNextChainTail = &m_descriptorBufferFeatures.pNext;
    }
    if (m_extensionCheck.HasVkShaderObject)
    {
        m_shaderObjectFeatures.shaderObject = VK_TRUE;
        *pNextChainTail = &m_shaderObjectFeatures;
        pNextChainTail = &m_shaderObjectFeatures.pNext;
    }
    if (m_extensionCheck.HasVkGraphicsPipelineLibrary)
    {
        m_graphicsPipelineLibraryFeatures.graphicsPipelineLibrary = VK_TRUE;
        *pNextChainTail = &m_graphicsPipelineLibraryFeatures;
        pNextChainTail = &m_graphicsPipelineLibraryFeatures.pNext;
    }
    if (m_extensionCheck.HasVkmesh_shader)
    {
        m_meshShaderFeatures.meshShader = VK_TRUE;
        m_meshShaderFeatures.taskShader = VK_TRUE;
        *pNextChainTail = &m_meshShaderFeatures;
        pNextChainTail = &m_meshShaderFeatures.pNext;
    }
    if (m_extensionCheck.HasVkAccelerationStructure)
    {
        m_accelerationStructureFeatures.accelerationStructure = VK_TRUE;
        *pNextChainTail = &m_accelerationStructureFeatures;
        pNextChainTail = &m_accelerationStructureFeatures.pNext;
    }
    if (m_extensionCheck.HasVkRayTracingPipeline)
    {
        m_rayTracingPipelineFeatures.rayTracingPipeline = VK_TRUE;
        *pNextChainTail = &m_rayTracingPipelineFeatures;
        pNextChainTail = &m_rayTracingPipelineFeatures.pNext;
    }
    if (m_extensionCheck.HasVkRayQuery)
    {
        m_rayQueryFeatures.rayQuery = VK_TRUE;
        *pNextChainTail = &m_rayQueryFeatures;
        pNextChainTail = &m_rayQueryFeatures.pNext;
    }
    if (m_extensionCheck.HasVkfragment_shading_rate)
    {
        m_fragmentShadingRateFeatures.pipelineFragmentShadingRate =
            m_supportedFragmentShadingRateFeatures.pipelineFragmentShadingRate;
        m_fragmentShadingRateFeatures.primitiveFragmentShadingRate =
            m_supportedFragmentShadingRateFeatures.primitiveFragmentShadingRate;
        m_fragmentShadingRateFeatures.attachmentFragmentShadingRate =
            m_supportedFragmentShadingRateFeatures.attachmentFragmentShadingRate;
        *pNextChainTail = &m_fragmentShadingRateFeatures;
        pNextChainTail = &m_fragmentShadingRateFeatures.pNext;
    }
    if (m_extensionCheck.HasVkCooperativeMatrix)
    {
        m_cooperativeMatrixFeatures.cooperativeMatrix = VK_TRUE;
        *pNextChainTail = &m_cooperativeMatrixFeatures;
        pNextChainTail = &m_cooperativeMatrixFeatures.pNext;
    }
    if (m_extensionCheck.HasVkRobustness2)
    {
        m_features2.features.robustBufferAccess = VK_TRUE;
        m_robustness2Features.robustBufferAccess2 = VK_TRUE;
        m_robustness2Features.robustImageAccess2 = VK_TRUE;
        m_robustness2Features.nullDescriptor = VK_TRUE;
        *pNextChainTail = &m_robustness2Features;
        pNextChainTail = &m_robustness2Features.pNext;
    }
    if (m_extensionCheck.HasVkMemoryPriority)
    {
        m_memoryPriorityFeatures.memoryPriority = VK_TRUE;
        *pNextChainTail = &m_memoryPriorityFeatures;
        pNextChainTail = &m_memoryPriorityFeatures.pNext;
    }
    if (m_extensionCheck.HasVkPageableDeviceLocalMemory)
    {
        m_pageableDeviceLocalMemoryFeatures.pageableDeviceLocalMemory = VK_TRUE;
        *pNextChainTail = &m_pageableDeviceLocalMemoryFeatures;
        pNextChainTail = &m_pageableDeviceLocalMemoryFeatures.pNext;
    }
    if (m_extensionCheck.HasVkHostImageCopy)
    {
        m_hostImageCopyFeatures.hostImageCopy = VK_TRUE;
        *pNextChainTail = &m_hostImageCopyFeatures;
        pNextChainTail = &m_hostImageCopyFeatures.pNext;
    }
    if (m_extensionCheck.HasVkDynamicRenderingLocalRead)
    {
        m_dynamicRenderingLocalReadFeatures.dynamicRenderingLocalRead = VK_TRUE;
        *pNextChainTail = &m_dynamicRenderingLocalReadFeatures;
        pNextChainTail = &m_dynamicRenderingLocalReadFeatures.pNext;
    }

    // NOTE: Extensions that don't need feature structs (just the extension string is enough):
    // VK_KHR_push_descriptor, VK_KHR_dedicated_allocation, VK_KHR_create_renderpass2,
    // VK_KHR_maintenance1/2/3, VK_EXT_memory_budget, VK_KHR_calibrated_timestamps,
    // VK_KHR_video_* (encode/decode), VK_ANDROID_external_memory_android_hardware_buffer

    *pNextChainTail = nullptr;
    return &m_features2;
};

void VulkanExtensionCheck::QuerySupportedFeatures(const vk::raii::PhysicalDevice& PhysicalDevice)
{
    if (!m_extensionCheck.HasVkfragment_shading_rate)
        return;

    vk::PhysicalDeviceFeatures2 queryFeatures2;
    m_supportedFragmentShadingRateFeatures.pNext = nullptr;
    queryFeatures2.pNext = &m_supportedFragmentShadingRateFeatures;
    vkGetPhysicalDeviceFeatures2(*PhysicalDevice, reinterpret_cast<VkPhysicalDeviceFeatures2*>(&queryFeatures2));
}

void VulkanExtensionCheck::TickTheAvailableExtension(const vk::raii::PhysicalDevice& PhysicalDevice)
{
    uint32_t propertyCount = 0;
    vkEnumerateDeviceExtensionProperties(*PhysicalDevice, nullptr, &propertyCount, nullptr);
    std::vector<VkExtensionProperties> properties(propertyCount);
    vkEnumerateDeviceExtensionProperties(*PhysicalDevice, nullptr, &propertyCount, properties.data());

    for (const auto& ext : properties)
    {
        std::string_view name = ext.extensionName;
        if (name == "VK_KHR_dynamic_rendering")
            m_extensionCheck.HasVkdynamicRendering = true;
        else if (name == "VK_KHR_synchronization2")
            m_extensionCheck.HasVkSync2 = true;
        else if (name == "VK_KHR_timeline_semaphore")
            m_extensionCheck.HasVkTimelineSemaphore = true;
        else if (name == "VK_KHR_push_descriptor")
            m_extensionCheck.HasVkPushDescriptor = true;
        else if (name == "VK_KHR_16bit_storage")
            m_extensionCheck.HasVK16bit_storage = true;
        else if (name == "VK_KHR_shader_float16_int8")
            m_extensionCheck.HasVkshader_float16_int8 = true;
        else if (name == "VK_KHR_buffer_device_address")
            m_extensionCheck.HasVkbuffer_device_address = true;
        else if (name == "VK_KHR_shader_subgroup_extended_types")
            m_extensionCheck.HasVkshader_subgroup_extended_types = true;
        else if (name == "VK_EXT_descriptor_indexing")
            m_extensionCheck.HasVkBindlessDescriptor = true;
        else if (name == "VK_EXT_descriptor_buffer")
            m_extensionCheck.HasVkDescriptorBuffer = true;
        else if (name == "VK_EXT_descriptor_heap")
            m_extensionCheck.HasVkDescriptorHeap = true;
        else if (name == "VK_EXT_shader_object")
            m_extensionCheck.HasVkShaderObject = true;
        else if (name == "VK_EXT_graphics_pipeline_library")
            m_extensionCheck.HasVkGraphicsPipelineLibrary = true;
        else if (name == "VK_EXT_extended_dynamic_state")
            m_extensionCheck.HasVkExtendedDynamicState = true;
        else if (name == "VK_EXT_extended_dynamic_state2")
            m_extensionCheck.HasVkExtendedDynamicState2 = true;
        else if (name == "VK_EXT_extended_dynamic_state3")
            m_extensionCheck.HasVkExtendedDynamicState3 = true;
        else if (name == "VK_KHR_draw_indirect_count")
            m_extensionCheck.HasVkDrawIndirectCount = true;
        else if (name == "VK_EXT_inline_uniform_block")
            m_extensionCheck.HasVkInlineUniformBlock = true;
        else if (name == "VK_KHR_shader_draw_parameters")
            m_extensionCheck.HasVkShaderDrawParameters = true;
        else if (name == "VK_EXT_mesh_shader")
            m_extensionCheck.HasVkmesh_shader = true;
        else if (name == "VK_EXT_device_generated_commands")
            m_extensionCheck.HasVkDeviceGeneratedCommands = true;
        else if (name == "VK_KHR_video_queue")
            m_extensionCheck.HasVkVideoQueue = true;
        else if (name == "VK_KHR_video_decode_av1")
            m_extensionCheck.HasVkdecode_av1 = true;
        else if (name == "VK_KHR_video_decode_h265")
            m_extensionCheck.HasVkdecode_h265 = true;
        else if (name == "VK_KHR_video_decode_h264")
            m_extensionCheck.HasVkdecode_h264 = true;
        else if (name == "VK_KHR_video_encode_h265")
            m_extensionCheck.HasVkencode_h265 = true;
        else if (name == "VK_KHR_video_encode_h264")
            m_extensionCheck.HasVkencode_h264 = true;
        else if (name == "VK_KHR_video_encode_av1")
        {
            m_extensionCheck.HasVkencode_av1 = true;
            m_extensionCheck.HasVkVideoEncodeAV1 = true;
        }
        else if (name == "VK_KHR_video_encode_feedback2")
            m_extensionCheck.HasVkVideoEncodeFeedback2 = true;
        else if (name == "VK_KHR_sampler_ycbcr_conversion")
            m_extensionCheck.HasVKYcbcr2Conversion = true;
        else if (name == "VK_ANDROID_external_memory_android_hardware_buffer")
            m_extensionCheck.HasVkExternalMemoryAndroidHardwareBuffer = true;
        else if (name == "VK_EXT_shader_framebuffer_fetch")
            m_extensionCheck.HasVkShaderFramebufferFetch = true;
        else if (name == "VK_EXT_memory_budget")
            m_extensionCheck.HasVkMemoryBudget = true;
        else if (name == "VK_EXT_texture_compression_astc_hdr")
            m_extensionCheck.HasVkTextureCompressionASTC_HDR = true;
        else if (name == "VK_EXT_memory_priority")
            m_extensionCheck.HasVkMemoryPriority = true;
        else if (name == "VK_EXT_pageable_device_local_memory")
            m_extensionCheck.HasVkPageableDeviceLocalMemory = true;
        else if (name == "VK_EXT_host_image_copy")
            m_extensionCheck.HasVkHostImageCopy = true;
        else if (name == "VK_KHR_dynamic_rendering_local_read")
            m_extensionCheck.HasVkDynamicRenderingLocalRead = true;
        else if (name == "VK_KHR_dedicated_allocation")
            m_extensionCheck.HasVkDedicatedAllocation = true;
        else if (name == "VK_KHR_acceleration_structure")
            m_extensionCheck.HasVkAccelerationStructure = true;
        else if (name == "VK_KHR_deferred_host_operations")
            m_extensionCheck.HasVkDeferredHostOperations = true;
        else if (name == "VK_KHR_ray_tracing_pipeline")
            m_extensionCheck.HasVkRayTracingPipeline = true;
        else if (name == "VK_KHR_ray_query")
            m_extensionCheck.HasVkRayQuery = true;
        else if (name == "VK_KHR_fragment_shading_rate")
            m_extensionCheck.HasVkfragment_shading_rate = true;
        else if (name == "VK_KHR_cooperative_matrix")
            m_extensionCheck.HasVkCooperativeMatrix = true;
        else if (name == "VK_EXT_robustness2")
            m_extensionCheck.HasVkRobustness2 = true;
        else if (name == "VK_EXT_image_robustness")
            m_extensionCheck.HasVkImageRobustness = true;
        else if (name == "VK_KHR_create_renderpass2")
            m_extensionCheck.HasVkCreateRenderPass2 = true;
    }
}

} // namespace UHE::RHI::VULKAN
