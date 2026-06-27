#include "uhepch.h"
#include "VulkanShader.h"
#include "UHE/Core/Log.h"

namespace UHE::RHI::VULKAN
{

void VulkanShader::Create(const vk::raii::Device& device, const ShaderDesc& desc)
{
    m_Stage = desc.stage;
    m_EntryPoint = desc.entryPoint;

    vk::ShaderModuleCreateInfo createInfo{
        .flags = {},
        .codeSize = desc.spirvSize,
        .pCode = reinterpret_cast<const uint32_t*>(desc.spirvData)
    };

    try
    {
        m_Module = vk::raii::ShaderModule(device, createInfo);
    }
    catch (const std::exception& e)
    {
        UHE_CORE_ERROR("Failed to create Vulkan Shader Module! Error: {0}", e.what());
        UHE_CORE_ASSERT(false, "Shader module creation failed!");
    }
    
    if (!*m_Module) {
        UHE_CORE_ASSERT(false, "Shader module is null after creation!");
    }
}

} // namespace UHE::RHI::VULKAN
