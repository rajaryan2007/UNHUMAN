#include "VulkanTypes.h"
#include "UHE/RHI/RHITypes.h"
#include "UHE/Renderer/Shader.h"
#include "vulkan/vulkan.hpp"

namespace UHE::RHI::VULKAN
{
vk::Format MapTextureFormat(TextureFormat format)
{
    switch (format)
    {
        case TextureFormat::RGBA8_UNORM:
            return vk::Format::eR8G8B8A8Unorm;
        case TextureFormat::RGBA8_SRGB:
            return vk::Format::eR8G8B8A8Srgb;
        case TextureFormat::BGRA8_UNORM:
            return vk::Format::eB8G8R8A8Unorm;
        case TextureFormat::BGRA8_SRGB:
            return vk::Format::eB8G8R8A8Srgb;
        case TextureFormat::D24_UNORM_S8:
            return vk::Format::eD32SfloatS8Uint;
        case TextureFormat::D32_FLOAT:
            return vk::Format::eD32Sfloat;
        case TextureFormat::R32_SINT:
            return vk::Format::eR32Sint;
        default:
            return vk::Format::eUndefined;
    }
}

vk::PrimitiveTopology MapTopology(PrimitiveTopology topology)
{
    switch (topology)
    {
        case PrimitiveTopology::TriangleList:
            return vk::PrimitiveTopology::eTriangleList;
        case PrimitiveTopology::TriangleStrip:
            return vk::PrimitiveTopology::eTriangleStrip;
        case PrimitiveTopology::LineList:
            return vk::PrimitiveTopology::eLineList;
        case PrimitiveTopology::PointList:
            return vk::PrimitiveTopology::ePointList;
    }
    return vk::PrimitiveTopology::eTriangleList;
}

vk::Format ShaderDataTypeToVulkanFormat(ShaderDataType type)
{
    switch (type)
    {
        case ShaderDataType::Float:
            return vk::Format::eR32Sfloat;
        case ShaderDataType::Float2:
            return vk::Format::eR32G32Sfloat;
        case ShaderDataType::Float3:
            return vk::Format::eR32G32B32Sfloat;
        case ShaderDataType::Float4:
            return vk::Format::eR32G32B32A32Sfloat;
        case ShaderDataType::Int:
            return vk::Format::eR32Sint;
        case ShaderDataType::Int2:
            return vk::Format::eR32G32Sint;
        case ShaderDataType::Int3:
            return vk::Format::eR32G32B32Sint;
        case ShaderDataType::Int4:
            return vk::Format::eR32G32B32A32Sint;
        case ShaderDataType::Mat3:
            return vk::Format::eR32G32B32Sfloat;
        case ShaderDataType::Mat4:
            return vk::Format::eR32G32B32A32Sfloat;
        case ShaderDataType::Bool:
            return vk::Format::eR32Uint;
        case ShaderDataType::None:
            return vk::Format::eUndefined;
    }
    return vk::Format::eUndefined;
}
} // namespace UHE::RHI::VULKAN
