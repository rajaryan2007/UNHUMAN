#pragma once
#include "UHE/Renderer/Texture.h"
#include "Platform/Vulkan/VulkanTexture.h"
#include <string>

namespace UHE {
    class VulkanTexture2D : public Texture2D {
    public:
        VulkanTexture2D(const std::string& path);
        VulkanTexture2D(u32 width, u32 height);
        VulkanTexture2D(const void* data, size_t size);
        virtual ~VulkanTexture2D() override;

        virtual u32 GetWidth() const override { return m_Width; }
        virtual u32 GetHeight() const override { return m_Height; }

        virtual void Bind(u32 slot = 0) const override {}
        virtual void* GetImGuiTextureID() override;
        virtual RHI::TextureHandle GetTextureHandle() const override { return reinterpret_cast<RHI::TextureHandle>(m_VulkanTexture.get()); }
        virtual u32 GetTextureIndex() const override { return m_VulkanTexture ? m_VulkanTexture->GetTextureIndex() : 0; }

        virtual bool operator==(const Texture& other) const override {
            return this == &other;
        }

    private:
        u32 m_Width = 0;
        u32 m_Height = 0;
        Ref<RHI::VULKAN::VulkanTexture> m_VulkanTexture;
    };
}
