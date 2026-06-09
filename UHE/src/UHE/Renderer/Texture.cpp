#include "uhepch.h"
#include "Texture.h"
#include "UHE/Renderer/Renderer.h"
#include "Platform/Vulkan/VulkanTexture2D.h"

namespace UHE {

    Ref<Texture2D> Texture2D::Create(const std::string& path)
    {
        return CreateRef<VulkanTexture2D>(path);
    }

    Ref<Texture2D> Texture2D::Create(u32 width, u32 height)
    {
        return CreateRef<VulkanTexture2D>(width, height);
    }

}
