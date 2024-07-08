#include "vulkan_renderer.hpp"
#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <algorithm>
#include <cassert>
namespace paranoixa {
VulkanRenderer::VulkanRenderer() {}
VulkanRenderer::~VulkanRenderer() {
  vkDestroyDevice(m_device, nullptr);
  vkDestroyInstance(m_instance, nullptr);
}
void VulkanRenderer::Initialize(void *window) {
  volkInitialize();
  CreateInstance(window);
  CreateDevice();
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
  {
    uint32_t count;
    const char *const *extensionNames =
        SDL_Vulkan_GetInstanceExtensions(&count);
    std::for_each_n(extensionNames, count,
                    [&](auto v) { extensions.push_back(v); });
  }
#ifdef _DEBUG
  layers.push_back("VK_LAYER_KHRONOS_validation");
#endif
  createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
  createInfo.ppEnabledExtensionNames = extensions.data();
  createInfo.enabledLayerCount = static_cast<uint32_t>(layers.size());
  createInfo.ppEnabledLayerNames = layers.data();
  VkResult result = vkCreateInstance(&createInfo, nullptr, &m_instance);
  if (result == VkResult::VK_SUCCESS) {
    volkLoadInstance(m_instance);
  }
}
void VulkanRenderer::CreateDevice() {
  //------------------------
  // Create physical device
  //------------------------
  uint32_t count = 0;
  vkEnumeratePhysicalDevices(m_instance, &count, nullptr);
  std::vector<VkPhysicalDevice> physicalDevices(count);
  vkEnumeratePhysicalDevices(m_instance, &count, physicalDevices.data());

  // Choose first
  m_physicalDevice = physicalDevices[0];

  vkGetPhysicalDeviceMemoryProperties(m_physicalDevice,
                                      &m_physicalDeviceMemoryProperties);

  vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &count, nullptr);
  std::vector<VkQueueFamilyProperties> queueFamilyProps(count);
  vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &count,
                                           queueFamilyProps.data());
  uint32_t gfxQueueIndex = ~0u;
  for (uint32_t i = 0; const auto &props : queueFamilyProps) {
    if (props.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
      gfxQueueIndex = i;
      break;
    }
    ++i;
  }
  assert(gfxQueueIndex != ~0u);
  m_graphicsQueueIndex = gfxQueueIndex;

  //------------------------
  // Create logic device
  //------------------------
  std::vector<const char *> extensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
  VkPhysicalDeviceFeatures2 physFeatures2{
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2};
  VkPhysicalDeviceVulkan13Features vulkan13Features{
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES};
  physFeatures2.pNext = &vulkan13Features;
  vulkan13Features.dynamicRendering = VK_TRUE;
  vulkan13Features.synchronization2 = VK_TRUE;
  vulkan13Features.maintenance4 = VK_TRUE;
  vkGetPhysicalDeviceFeatures2(m_physicalDevice, &physFeatures2);
  const float queuePriorities[] = {1.f};
  VkDeviceQueueCreateInfo deviceQueueCreateInfo{
      .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
      .queueFamilyIndex = m_graphicsQueueIndex,
      .queueCount = 1,
      .pQueuePriorities = queuePriorities};
  VkDeviceCreateInfo deviceCreateInfo{
      .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
      .queueCreateInfoCount = 1,
      .pQueueCreateInfos = &deviceQueueCreateInfo,
      .enabledExtensionCount = static_cast<uint32_t>(extensions.size()),
      .ppEnabledExtensionNames = extensions.data()};
  deviceCreateInfo.pNext = &physFeatures2;
  auto result =
      vkCreateDevice(m_physicalDevice, &deviceCreateInfo, nullptr, &m_device);
  if (result == VkResult::VK_SUCCESS) {

    volkLoadDevice(m_device);
    vkGetDeviceQueue(m_device, m_graphicsQueueIndex, 0, &m_graphicsQueue);
  }
}
} // namespace paranoixa