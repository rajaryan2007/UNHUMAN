#include "uhepch.h"
#include "VulkanBuffer.h"

namespace UHE::RHI {
 // ---------------------------------- Vulkan Buffer Abstraction -------------------------------
 
 VulkanBuffer::~VulkanBuffer() {}

 void VulkanBuffer::init(VmaAllocator allocator, vk::DeviceSize size,
                        vk::BufferUsageFlags usage,
                        VmaMemoryUsage memoryUsage) 
 {}

 void VulkanBuffer::UploadData(const void *data, vk::DeviceSize size) 
 {}

 void VulkanBuffer::CopyTo(VulkanBuffer &dstBuffer, vk::DeviceSize size,
                           vk::raii::CommandBuffer &commandBuffer) 
 {}

 // ---------------------------------- Vulkan Vertex Buffer Abstraction -------------------------------

 void VulkanVertexBuffer::Create(VmaAllocator allocator, const void *vertexData,
                                 uint32_t vertexCount, uint32_t stride) 
 {}

  // ---------------------------------- Vulkan Index Buffer Abstraction ------------------------------- 

 void VulkanIndexBuffer::Create(VmaAllocator allocator,
                                 const std::vector<uint32_t> &indices) 
 {}
 } // namespace UHE::RHI