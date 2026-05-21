#pragma once
#include "RHITypes.h"


namespace UHE::RHI {
	class RHISwapChain {
    public:
          virtual ~RHISwapChain() = default;

          // ─── Frame Sync & Presentation ───
          virtual void AcquireNextImage() = 0;
          virtual void Present() = 0;

          // ─── Window Manipulation ───
          virtual void ResizeSwapchain(u32 width, u32 height) = 0;

          // ─── Lookups ───
          virtual TextureHandle GetSwapchainImage() = 0;
          virtual TextureFormat GetSwapchainFormat() = 0;

	private:
    };
}
