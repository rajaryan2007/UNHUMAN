#pragma once
#include "UHE/Core/Core.h"
#include "UHE/RHI/RHIDevice.h"

namespace UHE {

class UHE_API Renderer {
public:
    static void Init();
    static void Shutdown();
    static void OnWindowResize(u32 width, u32 height);

    static RHI::RHIDevice& GetDevice();

private:
    static std::unique_ptr<RHI::RHIDevice> s_Device;
};

} // namespace UHE
