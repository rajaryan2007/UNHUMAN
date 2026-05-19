#include "uhepch.h"
#include "RHIDevice.h"


#include "Platform/Vulkan/VulkanDevice.h"

namespace UHE::RHI {

std::unique_ptr<RHIDevice> RHIDevice::Create(Backend backend, const SwapchainDesc& swapDesc) {
    switch (backend) {
    case Backend::Vulkan:
        return std::make_unique<VulkanDevice>(swapDesc);

    case Backend::DX12:
        VG_CORE_ASSERT(false, "DX12 backend not implemented yet!");
        return nullptr;

    case Backend::Metal:
        VG_CORE_ASSERT(false, "Metal backend not implemented yet!");
        return nullptr;

    case Backend::None:
    default:
        VG_CORE_ASSERT(false, "No rendering backend selected!");
        return nullptr;
    }
}

} // namespace UHE::RHI
