#pragma once

namespace UHE::RHI::VULKAN
{
class VulkanSemaphore;
struct SemaphoneSubmitInfo;
struct SubmitDesc;

class VulkanContext;

class VulkanBinarySemaphore
{
public:
    // @brief while it does support binary fence but it does support that much things
    // it very simple implementation of binary semaphore only gonna use in fallback
    // due to limitations discussed here: https://www.khronos.org/blog/vulkan-timeline-semaphores

    VulkanBinarySemaphore() = default;
    ~VulkanBinarySemaphore() = default;
    void init(VulkanContext* ctx);
    void Create();
    void Shutdown();

private:
};
} // namespace UHE::RHI::VULKAN
