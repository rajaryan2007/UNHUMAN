


namespace UHE::RHI {
class VulkanDevice;
class VulkanCommandBuffer {
public:
    VulkanCommandBuffer(const VulkanCommandBuffer &) = delete;
    VulkanCommandBuffer &operator=(const VulkanCommandBuffer &) = delete;

    void CreateCommandBuffer(); 
    void BeginCommandBuffer(VulkanDevice &device);
    void EndCommandBuffer();
  };
} // namespace UHE::RHI