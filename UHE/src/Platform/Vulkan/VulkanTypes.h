#pragma once
#include <vulkan/vulkan_raii.hpp>
#include "UHE/RHI/RHITypes.h"

namespace UHE::RHI::VULKAN
{

static vk::Format MapTextureFormat(TextureFormat format);
static vk::PrimitiveTopology MapTopology(PrimitiveTopology topology);
static vk::Format ShaderDataTypeToVulkanFormat(ShaderDataType type);

} // namespace UHE::RHI::VULKAN
