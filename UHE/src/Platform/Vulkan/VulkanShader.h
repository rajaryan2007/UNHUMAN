#pragma once
#include "UHE/RHI/RHITypes.h"
#include <vulkan/vulkan_raii.hpp>
#include <vector>

namespace UHE::RHI::VULKAN {

class VulkanShader {
public:
    VulkanShader() = default;
    
    // Create the shader module
    void Create(const vk::raii::Device& device, const ShaderDesc& desc);

    vk::ShaderModule GetHandle() const { return *m_Module; }
    ShaderStage GetStage() const { return m_Stage; }
    const char* GetEntryPoint() const { return m_EntryPoint.c_str(); }
    vk::ShaderModule GetModule() const { return *m_Module; }

private:
    vk::raii::ShaderModule m_Module;
    ShaderStage m_Stage = ShaderStage::Vertex;
    std::string m_EntryPoint = "main";
};

} // namespace UHE::RHI::VULKAN
