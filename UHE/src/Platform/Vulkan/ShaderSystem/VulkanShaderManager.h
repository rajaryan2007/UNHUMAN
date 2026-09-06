#pragma once


namespace UHE::RHI::VULKAN
{
class VulkanShaderManager {
    public:
    VulkanShaderManager() = default;
    ~VulkanShaderManager() = default;
    VulkanShaderManager(VulkanShaderManager&) = delete;
    VulkanShaderManager operator=(VulkanShaderManager&) = delete;
    VulkanShaderManager(const VulkanShaderManager&) = delete;
    VulkanShaderManager& operator=(const VulkanShaderManager&) = delete;
    VulkanShaderManager(VulkanShaderManager&&) = delete;
    VulkanShaderManager& operator=(VulkanShaderManager&&) = delete;
    
    void Init();
    void ShutDown();

    private:

};
}