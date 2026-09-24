#include "VulkanPipelineState.h"
#include <cstdint>
#include <type_traits>

namespace UHE::RHI::VULKAN
{
namespace
{
void HashAdd(u64& seed, u64 value)
{
    seed ^= value + 0x9e3779b97f4a7c15ULL + (seed << 6) + (seed >> 2);
}

void HashAdd(u64& seed, u32 value)
{
    HashAdd(seed, static_cast<u64>(value));
}

void HashAdd(u64& seed, bool value)
{
    HashAdd(seed, static_cast<u64>(value));
}

void HashAdd(u64& seed, const void* pointer)
{
    HashAdd(seed, static_cast<u64>(reinterpret_cast<uintptr_t>(pointer)));
}

template <typename T>
void HashAddEnum(u64& seed, T value)
{
    HashAdd(seed, static_cast<u64>(static_cast<std::underlying_type_t<T>>(value)));
}
} // namespace

u64 VulkanPipelineStateCache::Hash(const GraphicsPipelineDesc& desc)
{
    u64 seed = 0x243F6A8885A308D3ULL;

    HashAdd(seed, static_cast<const void*>(desc.vertexShader));
    HashAdd(seed, static_cast<const void*>(desc.fragmentShader));

    const auto& elements = desc.vertexLayout.GetElements();
    HashAdd(seed, static_cast<u64>(elements.size()));
    for (const auto& element : elements)
    {
        HashAddEnum(seed, element.Type);
        HashAdd(seed, element.Size);
        HashAdd(seed, element.Offset);
        HashAdd(seed, element.Normalized);
    }
    HashAdd(seed, desc.vertexLayout.GetStride());

    HashAddEnum(seed, desc.topology);
    HashAdd(seed, desc.colorAttachmentCount);
    for (u32 i = 0; i < desc.colorAttachmentCount; ++i)
        HashAddEnum(seed, desc.colorFormats[i]);
    HashAddEnum(seed, desc.depthFormat);
    HashAddEnum(seed, desc.blendMode);
    HashAdd(seed, desc.depthTest);
    HashAdd(seed, desc.depthWrite);
    HashAdd(seed, desc.pushConstantSize);

    const RenderPassDesc& renderPass = desc.renderPassDesc;
    HashAdd(seed, renderPass.colorAttachmentCount);
    for (u32 i = 0; i < renderPass.colorAttachmentCount; ++i)
    {
        const ColorAttachment& attachment = renderPass.colorAttachments[i];
        HashAddEnum(seed, attachment.loadOp);
        HashAddEnum(seed, attachment.storeOp);
        HashAddEnum(seed, attachment.format);
        HashAddEnum(seed, attachment.initialusage);
        HashAddEnum(seed, attachment.finalusage);
        HashAdd(seed, attachment.sampleCount);
        HashAddEnum(seed, attachment.stencilLoadOp);
        HashAddEnum(seed, attachment.stencilStoreOp);
    }

    HashAdd(seed, renderPass.subpassCount);
    for (u32 i = 0; i < renderPass.subpassCount; ++i)
    {
        const SubpassDesc& subpass = renderPass.subpasses[i];
        HashAdd(seed, subpass.colorAttachmentCount);
        for (u32 j = 0; j < subpass.colorAttachmentCount; ++j)
            HashAdd(seed, subpass.colorAttachments[j]);
        HashAdd(seed, subpass.depthAttachment);
    }

    HashAdd(seed, renderPass.dependencyCount);
    HashAdd(seed, renderPass.hasDepth);
    HashAddEnum(seed, renderPass.depthAttachment.loadOp);
    HashAddEnum(seed, renderPass.depthAttachment.storeOp);

    return seed;
}

u64 VulkanPipelineStateCache::Hash(const ComputePipelineDesc& desc)
{
    u64 seed = 0x9E3779B97F4A7C15ULL;

    HashAdd(seed, static_cast<const void*>(desc.computeShader));
    HashAdd(seed, desc.pushConstantSize);

    return seed;
}

PipelineHandle VulkanPipelineStateCache::Acquire(const GraphicsPipelineDesc& desc, const CreateFn& create)
{
    return AcquireHashed(Hash(desc), false, create);
}

PipelineHandle VulkanPipelineStateCache::Acquire(const ComputePipelineDesc& desc, const CreateFn& create)
{
    return AcquireHashed(Hash(desc), true, create);
}

PipelineHandle VulkanPipelineStateCache::AcquireHashed(u64 hash, bool compute, const CreateFn& create)
{
    auto* map = compute ? &m_Compute : &m_Graphics;
    std::unique_lock<std::mutex> lock(m_Mutex);

    for (;;)
    {
        auto it = map->find(hash);
        if (it == map->end())
        {
            (*map)[hash].pending = true;

            lock.unlock();
            PipelineHandle handle = nullptr;
            try
            {
                handle = create();
            }
            catch (...)
            {
                lock.lock();
                map->erase(hash);
                m_Cond.notify_all();
                throw;
            }
            lock.lock();

            auto ready = map->find(hash);
            if (ready == map->end())
                ready = map->emplace(hash, Entry{}).first;
            ready->second.handle = handle;
            ready->second.refCount = 1;
            ready->second.pending = false;
            m_Cond.notify_all();
            return handle;
        }

        if (it->second.pending)
        {
            m_Cond.wait(lock, [map, hash]() {
                auto pending = map->find(hash);
                return pending == map->end() || !pending->second.pending;
            });
            continue;
        }

        ++it->second.refCount;
        return it->second.handle;
    }
}

bool VulkanPipelineStateCache::Release(PipelineHandle handle)
{
    if (!handle)
        return false;

    std::unique_lock<std::mutex> lock(m_Mutex);

    const auto release = [&handle](std::unordered_map<u64, Entry>& map) {
        for (auto it = map.begin(); it != map.end(); ++it)
        {
            if (it->second.handle != handle)
                continue;
            if (--it->second.refCount == 0)
            {
                map.erase(it);
                return true;
            }
            return false;
        }
        return false;
    };

    return release(m_Graphics) || release(m_Compute);
}
} // namespace UHE::RHI::VULKAN
