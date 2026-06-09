#include "uhepch.h"
#include "VulkanShader.h"
#include "UHE/Core/Log.h"

namespace UHE::RHI::VULKAN
{

void VulkanShader::Create(const vk::raii::Device& device, const ShaderDesc& desc)
{
    m_Stage = desc.stage;
    m_EntryPoint = desc.entryPoint;

    vk::ShaderModuleCreateInfo createInfo{};
    createInfo.codeSize = desc.spirvSize;
    createInfo.pCode = reinterpret_cast<const uint32_t*>(desc.spirvData);

    try
    {
        m_Module = vk::raii::ShaderModule(device, createInfo);
    }
    catch (const std::exception& e)
    {
        UHE_CORE_ERROR("Failed to create Vulkan Shader Module! Error: {0}", e.what());
    }
}

} // namespace UHE::RHI::VULKAN
