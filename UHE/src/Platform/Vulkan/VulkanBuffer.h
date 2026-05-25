#pragma once 
#include <vulkan/vulkan_raii.hpp>

#include <vk_mem_alloc.h>


namespace UHE::RHI {

// ---------------------------------- Vulkan Buffer Abstraction -------------------------------

class VulkanBuffer {
public:

  ~VulkanBuffer();

  
  VulkanBuffer(const VulkanBuffer &) = delete;
  VulkanBuffer &operator=(const VulkanBuffer &) = delete;
 
  void init(VmaAllocator allocator, vk::DeviceSize size,
               vk::BufferUsageFlags usage, VmaMemoryUsage memoryUsage);
  void UploadData(const void *data, vk::DeviceSize size);
  void CopyTo(VulkanBuffer &dstBuffer, vk::DeviceSize size,
              vk::raii::CommandBuffer &commandBuffer);

  vk::Buffer GetHandle() const { return *m_Buffer; }

private:
  VmaAllocator m_Allocator = nullptr;
  vk::raii::Buffer m_Buffer = nullptr;
  VmaAllocation m_Allocation = nullptr;
  vk::DeviceSize m_Size = 0;
};

// ---------------------------------- Vulkan Vertex Buffer Abstraction ------------------------------- 

class VulkanVertexBuffer {
public:
  VulkanVertexBuffer(const VulkanVertexBuffer &) = delete;
  VulkanVertexBuffer &operator=(const VulkanVertexBuffer &) = delete;
  
  void Create(VmaAllocator allocator, const void *vertexData,
              uint32_t vertexCount, uint32_t stride);
  vk::Buffer GetHandle() const { return m_Buffer.GetHandle(); }
  uint32_t GetVertexCount() const { return m_VertexCount; }

private:
  VulkanBuffer m_Buffer;
  uint32_t m_VertexCount = 0;
};

// ---------------------------------- Vulkan Index Buffer Abstraction ------------------------------- 

class VulkanIndexBuffer {
public:
  VulkanIndexBuffer(const VulkanIndexBuffer &) = delete;
  VulkanIndexBuffer &operator=(const VulkanIndexBuffer &) = delete;  

  void Create(VmaAllocator allocator, const std::vector<uint32_t> &indices);
  vk::Buffer GetHandle() const { return m_Buffer.GetHandle(); }
  uint32_t GetIndexCount() const { return m_IndexCount; }

private:
  VulkanBuffer m_Buffer;
  uint32_t m_IndexCount = 0;
};

} // namespace UHE::RHI
