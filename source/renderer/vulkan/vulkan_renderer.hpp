#ifndef PARANOIXA_VULKAN_RENDERER_HPP
#define PARANOIXA_VULKAN_RENDERER_HPP
// Emscripten doesn't support Vulkan
#ifndef __EMSCRIPTEN__
#include "../renderer.hpp"

#include <vector>
#include <vulkan/vulkan.h>

#include "vma.hpp"

#include <cstdint>
#include <filesystem>
#include <memory>
namespace paranoixa {

class FileLoader {
public:
  bool Load(std::filesystem::path filePath, std::vector<char> &fileData);
};

std::unique_ptr<FileLoader> &GetFileLoader();
class VulkanRenderer : public Renderer {
public:
  VulkanRenderer();
  ~VulkanRenderer() override;
  void Initialize(void *window) override;
  void ProcessEvent(void *event) override;
  void Render() override;

private:
  void Finalize();
  void CreateInstance(void *window);
  void CreateDevice();
  void CreateSurface(void *window);
  void RecreateSwapchain(int width, int height);
  void CreateAllocator();
  void CreateCommandPool();
  void CreateDescriptorPool();
  void CreateSemaphores();
  void CreateCommandBuffers();
  void PrepareTriangle();
  void NewFrame();
  void ProcessFrame();
  void Submit();
  VkShaderModule CreateShaderModule(const void *code, size_t length);
  void DestroyShaderModule(VkShaderModule shaderModule);
  uint32_t GetMemoryTypeIndex(VkMemoryRequirements reqs,
                              VkMemoryPropertyFlags memoryPropFlags);
  void TransitionLayoutSwapchainImage(VkCommandBuffer commandBuffer,
                                      VkImageLayout newLayout,
                                      VkAccessFlags2 newAccessFlags);

  VkInstance instance;
  VkPhysicalDevice physicalDevice;
  VkPhysicalDeviceMemoryProperties physicalDeviceMemoryProperties;
  uint32_t graphicsQueueIndex;
  VkDevice device;
  VkQueue graphicsQueue;
  VkSurfaceKHR surface;
  VkSurfaceFormatKHR surfaceFormat;
  struct SwapchainState {
    VkImage image = VK_NULL_HANDLE;
    VkImageView view = VK_NULL_HANDLE;
    VkAccessFlags2 accessFlags = VK_ACCESS_2_NONE;
    VkImageLayout layout = VK_IMAGE_LAYOUT_UNDEFINED;
  };
  VkSwapchainKHR swapchain;
  std::vector<SwapchainState> swapchainState;
  VmaAllocator allocator;
  VkCommandPool commandPool;
  VkDescriptorPool descriptorPool;
  VkPipelineLayout pipelineLayout;
  VkPipeline pipeline;
  struct Frame {
    VkSemaphore renderCompleted;
    VkSemaphore presentCompleted;
    VkFence inFlightFence;
    VkCommandBuffer commandBuffer;
  };
  struct VertexBuffer {
    VkBuffer buffer;
    VmaAllocation memory;
  } vertexBuffer;
  int width, height;
  static constexpr size_t MAX_FRAMES_IN_FLIGHT = 2;
  Frame frames[MAX_FRAMES_IN_FLIGHT];
  int currentFrameIndex = 0;
  uint32_t swapchainImageIndex = 0;
};
VulkanRenderer &GetVulkanRenderer();
} // namespace paranoixa
#endif // __EMSCRIPTEN__
#endif // PARANOIXA_VULKAN_RENDERER_HPP