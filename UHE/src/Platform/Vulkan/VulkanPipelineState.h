#pragma once
#include <condition_variable>
#include <functional>
#include <mutex>
#include <unordered_map>
#include <vulkan/vulkan_raii.hpp>
#include "UHE/RHI/RHITypes.h"

namespace UHE::RHI::VULKAN
{
class VulkanPipelineState
{
public:
    VulkanPipelineState() = default;
    virtual ~VulkanPipelineState() = default;
    VulkanPipelineState(const VulkanPipelineState&) = delete;
    VulkanPipelineState& operator=(const VulkanPipelineState&) = delete;

    [[nodiscard]] virtual vk::Pipeline GetPipeline() const = 0;
    [[nodiscard]] virtual vk::PipelineLayout GetPipelineLayout() const = 0;
    [[nodiscard]] virtual vk::PipelineBindPoint GetBindPoint() const = 0;
};

class VulkanPipelineStateCache
{
public:
    using CreateFn = std::function<PipelineHandle()>;

    [[nodiscard]] PipelineHandle Acquire(const GraphicsPipelineDesc& desc, const CreateFn& create);
    [[nodiscard]] PipelineHandle Acquire(const ComputePipelineDesc& desc, const CreateFn& create);

    [[nodiscard]] bool Release(PipelineHandle handle);

    [[nodiscard]] static u64 Hash(const GraphicsPipelineDesc& desc);
    [[nodiscard]] static u64 Hash(const ComputePipelineDesc& desc);

private:
    struct Entry
    {
        PipelineHandle handle = nullptr;
        u32 refCount = 0;
        bool pending = false;
    };

    PipelineHandle AcquireHashed(u64 hash, bool compute, const CreateFn& create);

    std::mutex m_Mutex;
    std::condition_variable m_Cond;
    std::unordered_map<u64, Entry> m_Graphics;
    std::unordered_map<u64, Entry> m_Compute;
};
} // namespace UHE::RHI::VULKAN
