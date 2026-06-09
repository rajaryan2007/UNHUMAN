#include "uhepch.h"
#include "VulkanTexture2D.h"
#include "VulkanTexture.h"
#include "UHE/Renderer/Renderer.h"
#include "Platform/Vulkan/VulkanDevice.h"

#include <stb_image.h>

namespace UHE {

    VulkanTexture2D::VulkanTexture2D(const std::string& path)
    {
        int width, height, channels;
        stbi_set_flip_vertically_on_load(1);
        stbi_uc* data = stbi_load(path.c_str(), &width, &height, &channels, 4);

        if (data)
        {
            m_Width = width;
            m_Height = height;

            m_VulkanTexture = CreateRef<RHI::VULKAN::VulkanTexture>();
            auto& rhiDevice = Renderer::GetDevice();
            auto* vulkanDevice = static_cast<RHI::VULKAN::VulkanDevice*>(&rhiDevice);

            size_t dataSize = m_Width * m_Height * 4;
            m_VulkanTexture->CreateTexture(*vulkanDevice, data, m_Width, m_Height, dataSize);

            stbi_image_free(data);
        }
        else
        {
            UHE_CORE_ERROR("Failed to load image: {0}", path);
        }
    }

    VulkanTexture2D::VulkanTexture2D(u32 width, u32 height)
        : m_Width(width), m_Height(height)
    {
        m_VulkanTexture = CreateRef<RHI::VULKAN::VulkanTexture>();
        auto& rhiDevice = Renderer::GetDevice();
        auto* vulkanDevice = static_cast<RHI::VULKAN::VulkanDevice*>(&rhiDevice);

        size_t dataSize = m_Width * m_Height * 4;
        
        // For blank textures, allocate white pixel data
        u32* whiteData = new u32[m_Width * m_Height];
        for (u32 i = 0; i < m_Width * m_Height; i++) whiteData[i] = 0xffffffff;

        m_VulkanTexture->CreateTexture(*vulkanDevice, whiteData, m_Width, m_Height, dataSize);
        delete[] whiteData;
    }

    VulkanTexture2D::~VulkanTexture2D()
    {
        // VulkanTexture destructor or cleanup will handle resources
    }

    void* VulkanTexture2D::GetImGuiTextureID()
    {
        if (m_VulkanTexture)
            return m_VulkanTexture->GetImGuiTextureID();
        return nullptr;
    }

}
