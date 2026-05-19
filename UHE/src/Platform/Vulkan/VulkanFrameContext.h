#pragma once
#include <vulkan/vulkan_raii.hpp>
#include "UHE/RHI/DeletionQueue.h"

namespace UHE::RHI {

class VulkanFrameContext {
public:
    VulkanFrameContext() = default;
    
    
    void Init(const vk::raii::Device& device, u32 queueFamilyIndex);
    
    
    void Cleanup();

   private:
    vk::raii::CommandPool   commandPool = nullptr;
    vk::raii::CommandBuffer commandBuffer = nullptr;
    vk::raii::Semaphore     imageAvailableSemaphore = nullptr;
    vk::raii::Semaphore     renderFinishedSemaphore = nullptr;
    vk::raii::Fence         inFlightFence = nullptr;
    
    // Deferred resource destruction queue per-frame
    DeletionQueue           deletionQueue;
};

} // namespace UHE::RHI
