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

  VkInstance m_instance;
  VkPhysicalDevice m_physicalDevice;
  VkPhysicalDeviceMemoryProperties m_physicalDeviceMemoryProperties;
  uint32_t m_graphicsQueueIndex;
  VkDevice m_device;
  VkQueue m_graphicsQueue;
};
} // namespace paranoixa
#endif // __EMSCRIPTEN__
#endif // PARANOIXA_VULKAN_RENDERER_HPP