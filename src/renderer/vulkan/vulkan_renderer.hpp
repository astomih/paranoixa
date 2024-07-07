#ifndef PARANOIXA_VULKAN_RENDERER_HPP
#define PARANOIXA_VULKAN_RENDERER_HPP
// Emscripten doesn't support Vulkan
#ifndef __EMSCRIPTEN__
#include "../renderer.hpp"

#include <volk.h>
#include <vulkan/vulkan.h>
namespace paranoixa {
class VulkanRenderer : public Renderer {
public:
  VulkanRenderer();
  ~VulkanRenderer();
  void Initialize(void *window) override;
  void Render() override;

private:
  void CreateInstance(void *window);
};
} // namespace paranoixa
#endif // __EMSCRIPTEN__
#endif // PARANOIXA_VULKAN_RENDERER_HPP