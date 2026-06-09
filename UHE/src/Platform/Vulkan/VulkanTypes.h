#pragma once
#include <vulkan/vulkan_raii.hpp>
#include "UHE/RHI/RHITypes.h"

namespace UHE::RHI::VULKAN
{

vk::Format MapTextureFormat(TextureFormat format);
vk::PrimitiveTopology MapTopology(PrimitiveTopology topology);
vk::Format ShaderDataTypeToVulkanFormat(ShaderDataType type);

} // namespace UHE::RHI::VULKAN
