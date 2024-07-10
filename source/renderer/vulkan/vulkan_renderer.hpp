#ifndef PARANOIXA_VULKAN_RENDERER_HPP
#define PARANOIXA_VULKAN_RENDERER_HPP
// Emscripten doesn't support Vulkan
#ifndef __EMSCRIPTEN__
#include "../renderer.hpp"

#include <vector>
#include <volk.h>
#include <vulkan/vulkan.h>
namespace paranoixa {
class VulkanRenderer : public Renderer {
public:
  VulkanRenderer();
  ~VulkanRenderer() override;
  void Initialize(void *window) override;
  void Render() override;

private:
  void CreateInstance(void *window);
  void CreateDevice();

  VkInstance instance;
  VkPhysicalDevice physicalDevice;
  VkPhysicalDeviceMemoryProperties physicalDeviceMemoryProperties;
  uint32_t graphicsQueueIndex;
  VkDevice device;
  VkQueue graphicsQueue;
};
} // namespace paranoixa
#endif // __EMSCRIPTEN__
#endif // PARANOIXA_VULKAN_RENDERER_HPP