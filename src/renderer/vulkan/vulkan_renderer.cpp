#include "vulkan_renderer.hpp"
#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <vector>
namespace paranoixa {
VulkanRenderer::VulkanRenderer() {}
VulkanRenderer::~VulkanRenderer() {}
void VulkanRenderer::Initialize(void *window) {
  volkInitialize();
  CreateInstance(window);
  // CreateAdapter();
  // CreateDevice();
  // CreateQueue();
  // CreateSurface(window);
  // ConfigSurface();
  // InitializePipeline();
}
void VulkanRenderer::Render() {}

void VulkanRenderer::CreateInstance(void *window) {
  auto *sdlWindow = static_cast<SDL_Window *>(window);
  const char *appName = SDL_GetWindowTitle(sdlWindow);
  VkApplicationInfo appInfo = {
      .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
      .pNext = nullptr,
      .pApplicationName = appName,
      .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
      .pEngineName = "Paranoixa",
      .engineVersion = VK_MAKE_VERSION(1, 0, 0),
      .apiVersion = VK_API_VERSION_1_3,
  };
  VkInstanceCreateInfo createInfo = {
      .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .pApplicationInfo = &appInfo,
      .enabledLayerCount = 0,
      .ppEnabledLayerNames = nullptr,
      .enabledExtensionCount = 0,
      .ppEnabledExtensionNames = nullptr,
  };
  std::vector<const char *> layers;
  std::vector<const char *> extensions;
  uint32_t layerCount;
  /*
  SDL_Vulkan_GetInstanceExtensions(sdlWindow, &layerCount,
                                                                   (const char
  **)nullptr);
                                                                   */
}
} // namespace paranoixa