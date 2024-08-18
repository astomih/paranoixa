#include "imgui.h"

#define IMGUI_IMPL_VULKAN_HAS_DYNAMIC_RENDERING
#define IMGUI_IMPL_VULKAN_USE_VOLK
#define VK_NO_PROTOTYPES
#include "backends/imgui_impl_vulkan.h"

#include "backends/imgui_impl_sdl3.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>

#define VOLK_IMPLEMENTATION
#include <volk.h>

#include <algorithm>
#include <array>
#include <cassert>
#include <fstream>

#include "vulkan_renderer.hpp"
namespace paranoixa {

static std::unique_ptr<FileLoader> gFileLoader = nullptr;

std::unique_ptr<FileLoader> &GetFileLoader() {
  if (gFileLoader == nullptr) {
    gFileLoader = std::make_unique<FileLoader>();
  }
  return gFileLoader;
}

bool FileLoader::Load(std::filesystem::path filePath,
                      std::vector<char> &fileData) {
  if (std::filesystem::exists(filePath)) {
    std::ifstream infile(filePath, std::ios::binary);
    if (infile) {
      auto size = infile.seekg(0, std::ios::end).tellg();
      fileData.resize(size);
      infile.seekg(0, std::ios::beg).read(fileData.data(), size);
      return true;
    }
  }
  filePath = std::filesystem::path("../") / filePath;
  if (std::filesystem::exists(filePath)) {
    std::ifstream infile(filePath, std::ios::binary);
    if (infile) {
      auto size = infile.seekg(0, std::ios::end).tellg();
      fileData.resize(size);
      infile.seekg(0, std::ios::beg).read(fileData.data(), size);
      return true;
    }
  }
  return false;
}
VulkanRenderer::VulkanRenderer()
    : device(VK_NULL_HANDLE), graphicsQueue(VK_NULL_HANDLE),
      instance(VK_NULL_HANDLE), physicalDevice(VK_NULL_HANDLE),
      surface(VK_NULL_HANDLE), graphicsQueueIndex(0), swapchain(VK_NULL_HANDLE),
      commandPool(VK_NULL_HANDLE), descriptorPool(VK_NULL_HANDLE),
      surfaceFormat(), frames(), pipelineLayout(VK_NULL_HANDLE),
      pipeline(VK_NULL_HANDLE), vertexBuffer(), swapchainState(),
      swapchainImageIndex(0), currentFrameIndex(0), width(0), height(0),
      physicalDeviceMemoryProperties() {}
VulkanRenderer::~VulkanRenderer() { Finalize(); }
void VulkanRenderer::Finalize() {
  vkDeviceWaitIdle(device);

  ImGui_ImplVulkan_Shutdown();
  ImGui_ImplSDL3_Shutdown();
  ImGui::DestroyContext();
  vkDestroyBuffer(device, vertexBuffer.buffer, nullptr);
  vkFreeMemory(device, vertexBuffer.memory, nullptr);
  for (auto &state : swapchainState) {
    vkDestroyImageView(device, state.view, nullptr);
  }
  vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
  vkDestroyPipeline(device, pipeline, nullptr);
  for (auto &frame : frames) {
    vkDestroyFence(device, frame.inFlightFence, nullptr);
    vkDestroySemaphore(device, frame.presentCompleted, nullptr);
    vkDestroySemaphore(device, frame.renderCompleted, nullptr);
  }
  vkDestroyDescriptorPool(device, descriptorPool, nullptr);
  vkDestroyCommandPool(device, commandPool, nullptr);
  vkDestroySwapchainKHR(device, swapchain, nullptr);
  SDL_Vulkan_DestroySurface(instance, surface, nullptr);
  vkDestroyDevice(device, nullptr);
  vkDestroyInstance(instance, nullptr);
}
void VulkanRenderer::Initialize(void *window) {
  auto *sdlWindow = static_cast<SDL_Window *>(window);
  SDL_GetWindowSize(sdlWindow, &width, &height);
  volkInitialize();
  CreateInstance(window);
  CreateDevice();
  CreateSurface(window);

  RecreateSwapchain(width, height);

  CreateCommandPool();
  CreateDescriptorPool();
  CreateSemaphores();
  CreateCommandBuffers();

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  (void)io;
  io.ConfigFlags |=
      ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad; // Enable Gamepad Control
  io.WantCaptureMouse = true;
  ImGui::StyleColorsDark();
  ImGui_ImplSDL3_InitForVulkan(sdlWindow);
  ImGui_ImplVulkan_LoadFunctions([](const char *functionName, void *userArgs) {
    auto &dev = GetVulkanRenderer();
    auto vkDevice = dev.device;
    auto vkInstance = dev.instance;
    auto devFuncAddr = vkGetDeviceProcAddr(vkDevice, functionName);
    if (devFuncAddr != nullptr) {
      return devFuncAddr;
    }
    auto instanceFuncAddr = vkGetInstanceProcAddr(vkInstance, functionName);
    return instanceFuncAddr;
  });
  ImGui_ImplVulkan_InitInfo vulkanInfo{
      .Instance = instance,
      .PhysicalDevice = physicalDevice,
      .Device = device,
      .QueueFamily = graphicsQueueIndex,
      .Queue = graphicsQueue,
      .DescriptorPool = descriptorPool,
      .RenderPass = VK_NULL_HANDLE,
      .MinImageCount = MAX_FRAMES_IN_FLIGHT,
      .ImageCount = MAX_FRAMES_IN_FLIGHT,
      .MSAASamples = VK_SAMPLE_COUNT_1_BIT,
  };
  vulkanInfo.UseDynamicRendering = true;
  vulkanInfo.PipelineRenderingCreateInfo = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
      .colorAttachmentCount = 1,
      .pColorAttachmentFormats = &surfaceFormat.format,
      .depthAttachmentFormat = VK_FORMAT_UNDEFINED,
  };

  ImGui_ImplVulkan_Init(&vulkanInfo);
  PrepareTriangle();
}
void VulkanRenderer::ProcessEvent(void *event) {
  ImGui_ImplSDL3_ProcessEvent(static_cast<SDL_Event *>(event));
}
void VulkanRenderer::Render() {
  this->NewFrame();
  this->ProcessFrame();
}
void VulkanRenderer::NewFrame() {
  auto &frameInfo = this->frames[currentFrameIndex];
  auto fence = frameInfo.inFlightFence;
  vkWaitForFences(device, 1, &fence, VK_TRUE, UINT64_MAX);
  auto res = vkAcquireNextImageKHR(device, swapchain, UINT64_MAX,
                                   frameInfo.presentCompleted, VK_NULL_HANDLE,
                                   &swapchainImageIndex);
  if (res == VK_ERROR_OUT_OF_DATE_KHR) {
    return;
  }
  vkResetFences(device, 1, &fence);

  vkResetCommandBuffer(frameInfo.commandBuffer, 0);
  VkCommandBufferBeginInfo commandBeginInfo{
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
  };
  vkBeginCommandBuffer(frameInfo.commandBuffer, &commandBeginInfo);
}
void VulkanRenderer::ProcessFrame() {
  auto commandBuffer = this->frames[this->currentFrameIndex].commandBuffer;

  VkClearValue clearValue = {
      VkClearColorValue{1.0f, 0.4f, 0.0f, 1.0f},
  };

  TransitionLayoutSwapchainImage(commandBuffer,
                                 VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                 VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT);
  VkRenderingAttachmentInfo colorAttachmentInfo{
      .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
      .imageView = this->swapchainState[this->swapchainImageIndex].view,
      .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
      .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
      .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
      .clearValue = clearValue,
  };
  VkRenderingInfo renderingInfo{
      .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
      .renderArea =
          {
              .extent = {uint32_t(width), uint32_t(height)},
          },
      .layerCount = 1,
      .colorAttachmentCount = 1,
      .pColorAttachments = &colorAttachmentInfo,
  };

  vkCmdBeginRendering(commandBuffer, &renderingInfo);

  vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
  VkDeviceSize vertexOffsets[] = {0};
  vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertexBuffer.buffer,
                         vertexOffsets);
  vkCmdDraw(commandBuffer, 3, 1, 0, 0);

  ImGui_ImplSDL3_NewFrame();
  ImGui_ImplVulkan_NewFrame();
  ImGui::NewFrame();
  ImGui::ShowDemoWindow();
  ImGui::Begin("Hello, world!");
  ImGui::End();
  ImGui::Render();
  ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);

  vkCmdEndRendering(commandBuffer);

  TransitionLayoutSwapchainImage(commandBuffer, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                                 VK_ACCESS_2_NONE);
  Submit();
}
void VulkanRenderer::Submit() {
  auto &frameInfo = frames[currentFrameIndex];
  vkEndCommandBuffer(frameInfo.commandBuffer);

  VkPipelineStageFlags waitStage{VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
  VkSubmitInfo submitInfo{
      .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
      .waitSemaphoreCount = 1,
      .pWaitSemaphores = &frameInfo.presentCompleted,
      .pWaitDstStageMask = &waitStage,
      .commandBufferCount = 1,
      .pCommandBuffers = &frameInfo.commandBuffer,
      .signalSemaphoreCount = 1,
      .pSignalSemaphores = &frameInfo.renderCompleted,
  };
  vkQueueSubmit(graphicsQueue, 1, &submitInfo, frameInfo.inFlightFence);

  currentFrameIndex = (++currentFrameIndex) % this->MAX_FRAMES_IN_FLIGHT;

  VkPresentInfoKHR presentInfo{.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
                               .waitSemaphoreCount = 1,
                               .pWaitSemaphores = &frameInfo.renderCompleted,
                               .swapchainCount = 1,
                               .pSwapchains = &swapchain,
                               .pImageIndices = &swapchainImageIndex};
  vkQueuePresentKHR(graphicsQueue, &presentInfo);
}

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
  VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);
  if (result == VkResult::VK_SUCCESS) {
    volkLoadInstance(instance);
  }
}
void VulkanRenderer::CreateDevice() {
  //------------------------
  // Create physical device
  //------------------------
  uint32_t count = 0;
  vkEnumeratePhysicalDevices(instance, &count, nullptr);
  std::vector<VkPhysicalDevice> physicalDevices(count);
  vkEnumeratePhysicalDevices(instance, &count, physicalDevices.data());

  // Choose first
  physicalDevice = physicalDevices[0];

  vkGetPhysicalDeviceMemoryProperties(physicalDevice,
                                      &physicalDeviceMemoryProperties);

  vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &count, nullptr);
  std::vector<VkQueueFamilyProperties> queueFamilyProps(count);
  vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &count,
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
  graphicsQueueIndex = gfxQueueIndex;

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
  vkGetPhysicalDeviceFeatures2(physicalDevice, &physFeatures2);
  const float queuePriorities[] = {1.f};
  VkDeviceQueueCreateInfo deviceQueueCreateInfo{
      .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
      .queueFamilyIndex = graphicsQueueIndex,
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
      vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &device);
  if (result == VkResult::VK_SUCCESS) {
    volkLoadDevice(device);
    vkGetDeviceQueue(device, graphicsQueueIndex, 0, &graphicsQueue);
  }
}
void VulkanRenderer::CreateSurface(void *window) {
  SDL_Vulkan_CreateSurface((SDL_Window *)window, this->instance, nullptr,
                           &this->surface);
  // Select format
  uint32_t count = 0;
  vkGetPhysicalDeviceSurfaceFormatsKHR(this->physicalDevice, this->surface,
                                       &count, nullptr);
  std::vector<VkSurfaceFormatKHR> formats(count);
  vkGetPhysicalDeviceSurfaceFormatsKHR(this->physicalDevice, this->surface,
                                       &count, formats.data());

  const VkFormat desireFormats[] = {VK_FORMAT_B8G8R8A8_UNORM,
                                    VK_FORMAT_R8G8B8A8_UNORM};
  this->surfaceFormat.format = VK_FORMAT_UNDEFINED;
  bool found = false;
  for (int i = 0; i < std::size(desireFormats) && !found; ++i) {
    auto format = desireFormats[i];
    for (const auto &f : formats) {
      if (f.format == format &&
          f.colorSpace == VK_COLORSPACE_SRGB_NONLINEAR_KHR) {
        this->surfaceFormat = f;
        found = true;
        break;
      }
    }
  }
  assert(found);
}
uint32_t
VulkanRenderer::GetMemoryTypeIndex(VkMemoryRequirements reqs,
                                   VkMemoryPropertyFlags memoryPropFlags) {
  auto requestBits = reqs.memoryTypeBits;
  for (uint32_t i = 0; i < physicalDeviceMemoryProperties.memoryTypeCount;
       ++i) {
    if (requestBits & 1) {
      const auto types = physicalDeviceMemoryProperties.memoryTypes[i];
      if ((types.propertyFlags & memoryPropFlags) == memoryPropFlags) {
        return i;
      }
    }
    requestBits >>= 1;
  }
  return UINT32_MAX;
}

void VulkanRenderer::RecreateSwapchain(int width, int height) {
  VkSurfaceCapabilitiesKHR surfaceCapabilities;
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface,
                                            &surfaceCapabilities);
  VkExtent2D extent = surfaceCapabilities.currentExtent;
  if (extent.width == UINT32_MAX) {
    extent.width = std::clamp(static_cast<uint32_t>(width),
                              surfaceCapabilities.minImageExtent.width,
                              surfaceCapabilities.maxImageExtent.width);
    extent.height = std::clamp(static_cast<uint32_t>(height),
                               surfaceCapabilities.minImageExtent.height,
                               surfaceCapabilities.maxImageExtent.height);
  }
  VkSwapchainKHR oldSwapchain = this->swapchain;
  VkSwapchainCreateInfoKHR swapchainCreateInfo{
      .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
      .surface = surface,
      .minImageCount = surfaceCapabilities.minImageCount,
      .imageFormat = surfaceFormat.format,
      .imageColorSpace = surfaceFormat.colorSpace,
      .imageExtent = extent,
      .imageArrayLayers = 1,
      .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
      .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
      .queueFamilyIndexCount = 0,
      .pQueueFamilyIndices = nullptr,
      .preTransform = surfaceCapabilities.currentTransform,
      .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
      .presentMode = VK_PRESENT_MODE_FIFO_KHR,
      .clipped = VK_TRUE,
      .oldSwapchain = oldSwapchain};
  vkCreateSwapchainKHR(device, &swapchainCreateInfo, nullptr, &this->swapchain);

  uint32_t count = 0;
  vkGetSwapchainImagesKHR(this->device, this->swapchain, &count, nullptr);
  std::vector<VkImage> swapchainImages(count);
  vkGetSwapchainImagesKHR(this->device, this->swapchain, &count,
                          swapchainImages.data());

  std::vector<SwapchainState> swapchainState(count);

  for (auto i = 0; auto &state : swapchainState) {
    auto image = swapchainImages[i];
    VkImageViewCreateInfo imageViewCI{
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = image,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = surfaceFormat.format,
        .components =
            {
                VK_COMPONENT_SWIZZLE_IDENTITY,
                VK_COMPONENT_SWIZZLE_IDENTITY,
                VK_COMPONENT_SWIZZLE_IDENTITY,
                VK_COMPONENT_SWIZZLE_IDENTITY,
            },
        .subresourceRange = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1,
        }};
    vkCreateImageView(device, &imageViewCI, nullptr, &state.view);
    state.image = image;
    state.layout = VK_IMAGE_LAYOUT_UNDEFINED;
    state.accessFlags = VK_ACCESS_2_NONE;
    ++i;
  }
  swapchainState.swap(this->swapchainState);

  if (oldSwapchain != VK_NULL_HANDLE) {
    for (auto &state : swapchainState) {
      vkDestroyImageView(device, state.view, nullptr);
    }
    vkDestroySwapchainKHR(device, oldSwapchain, nullptr);
  }

  count = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &count, nullptr);
  std::vector<VkBool32> supportPresent(count);
  for (uint32_t i = 0; i < count; ++i) {
    vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, this->surface,
                                         &supportPresent[i]);
  }
  assert(supportPresent[graphicsQueueIndex] == VK_TRUE);
}
void VulkanRenderer::CreateCommandPool() {
  VkCommandPoolCreateInfo commandPoolCreateInfo{
      .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
      .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
      .queueFamilyIndex = graphicsQueueIndex};
  VkCommandPool commandPool;
  vkCreateCommandPool(device, &commandPoolCreateInfo, nullptr,
                      &this->commandPool);
}
void VulkanRenderer::CreateDescriptorPool() {
  constexpr uint32_t POOL_SIZE = 10000;
  VkDescriptorPoolSize poolSizes[] = {
      {.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, .descriptorCount = POOL_SIZE},
      {.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
       .descriptorCount = POOL_SIZE}};
  VkDescriptorPoolCreateInfo descriptorPoolCreateInfo{
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
      .maxSets = 1,
      .poolSizeCount = std::size(poolSizes),
      .pPoolSizes = poolSizes};
  vkCreateDescriptorPool(device, &descriptorPoolCreateInfo, nullptr,
                         &this->descriptorPool);
}
void VulkanRenderer::CreateSemaphores() {
  VkSemaphoreCreateInfo semaphoreCreateInfo{
      .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
  for (auto &frame : frames) {
    vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr,
                      &frame.renderCompleted);
    vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr,
                      &frame.presentCompleted);
  }
}
void VulkanRenderer::CreateCommandBuffers() {
  VkFenceCreateInfo fenceCI{
      .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
      .flags = VK_FENCE_CREATE_SIGNALED_BIT,
  };
  for (auto &frame : this->frames) {
    vkCreateFence(device, &fenceCI, nullptr, &frame.inFlightFence);
  }
  VkCommandBufferAllocateInfo commandBufferAllocateInfo{
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
      .commandPool = commandPool,
      .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
      .commandBufferCount = static_cast<uint32_t>(std::size(frames))};
  std::vector<VkCommandBuffer> commandBuffers(std::size(frames));
  vkAllocateCommandBuffers(device, &commandBufferAllocateInfo,
                           commandBuffers.data());
  for (size_t i = 0; i < std::size(frames); ++i) {
    frames[i].commandBuffer = commandBuffers[i];
  }
}
void VulkanRenderer::PrepareTriangle() {
  float triangleVerts[] = {0.5f,  0.5f,  0.5f, 0, 0, 1,
                           0.0f,  -0.5f, 0.5f, 0, 1, 0,
                           -0.5f, 0.5f,  0.5f, 1, 0, 0};
  VkBufferCreateInfo bufferCI{
      .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
      .size = uint32_t(sizeof(triangleVerts)),
      .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
  };
  vkCreateBuffer(device, &bufferCI, nullptr, &vertexBuffer.buffer);
  VkMemoryRequirements reqs;
  vkGetBufferMemoryRequirements(device, vertexBuffer.buffer, &reqs);
  VkMemoryAllocateInfo memoryAllocateInfo{
      .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
      .allocationSize = reqs.size,
      .memoryTypeIndex =
          GetMemoryTypeIndex(reqs, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                       VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)};
  vkAllocateMemory(device, &memoryAllocateInfo, nullptr, &vertexBuffer.memory);
  vkBindBufferMemory(device, vertexBuffer.buffer, vertexBuffer.memory, 0);

  void *mapped;
  vkMapMemory(device, vertexBuffer.memory, 0, VK_WHOLE_SIZE, 0, &mapped);
  memcpy(mapped, triangleVerts, sizeof(triangleVerts));
  vkUnmapMemory(device, vertexBuffer.memory);

  VkPipelineLayoutCreateInfo layoutCI{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
  };
  vkCreatePipelineLayout(device, &layoutCI, nullptr, &pipelineLayout);

  VkVertexInputBindingDescription vertexBindingDesc{
      .binding = 0,
      .stride = uint32_t(sizeof(float)) * 6,
      .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
  };
  VkVertexInputAttributeDescription vertexAttribs[] = {
      {.location = 0,
       .binding = 0,
       .format = VK_FORMAT_R32G32B32_SFLOAT,
       .offset = 0},
      {.location = 1,
       .binding = 0,
       .format = VK_FORMAT_R32G32B32_SFLOAT,
       .offset = sizeof(float) * 3},
  };
  VkPipelineVertexInputStateCreateInfo vertexInput{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
      .vertexBindingDescriptionCount = 1,
      .pVertexBindingDescriptions = &vertexBindingDesc,
      .vertexAttributeDescriptionCount = 2,
      .pVertexAttributeDescriptions = vertexAttribs,
  };

  VkPipelineInputAssemblyStateCreateInfo inputAssembly{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
      .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST};

  VkViewport viewport{
      .x = 0,
      .y = 0,
      .width = float(width),
      .height = float(height),
      .minDepth = 0.0f,
      .maxDepth = 1.0f,
  };
  VkRect2D scissor{
      .offset = {0, 0},
      .extent = {.width = uint32_t(width), .height = uint32_t(height)},
  };

  VkPipelineViewportStateCreateInfo viewportState{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
      .viewportCount = 1,
      .pViewports = &viewport,
      .scissorCount = 1,
      .pScissors = &scissor,
  };

  VkPipelineRasterizationStateCreateInfo rasterizeState{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
      .cullMode = VK_CULL_MODE_BACK_BIT,
      .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
      .lineWidth = 1.0f,
  };

  VkPipelineMultisampleStateCreateInfo multisampleState{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
      .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
  };

  VkPipelineColorBlendAttachmentState blendAttachment{
      .blendEnable = VK_TRUE,
      .srcColorBlendFactor = VK_BLEND_FACTOR_ONE,
      .dstColorBlendFactor = VK_BLEND_FACTOR_ZERO,
      .colorBlendOp = VK_BLEND_OP_ADD,
      .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
      .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
      .alphaBlendOp = VK_BLEND_OP_ADD,
      .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
  };

  VkPipelineColorBlendStateCreateInfo blendState{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
      .attachmentCount = 1,
      .pAttachments = &blendAttachment};

  VkPipelineDepthStencilStateCreateInfo depthStencilState{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO};

  std::vector<char> vertexShaderSpv, fragmentShaderSpv;
  GetFileLoader()->Load("res/shader.vert.spv", vertexShaderSpv);
  GetFileLoader()->Load("res/shader.frag.spv", fragmentShaderSpv);

  std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages{
      {{
           .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
           .stage = VK_SHADER_STAGE_VERTEX_BIT,
           .module = CreateShaderModule(vertexShaderSpv.data(),
                                        vertexShaderSpv.size()),
           .pName = "main",
       },
       {
           .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
           .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
           .module = CreateShaderModule(fragmentShaderSpv.data(),
                                        fragmentShaderSpv.size()),
           .pName = "main",
       }}};

  VkGraphicsPipelineCreateInfo pipelineCreateInfo{
      .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
      .stageCount = uint32_t(shaderStages.size()),
      .pStages = shaderStages.data(),
      .pVertexInputState = &vertexInput,
      .pInputAssemblyState = &inputAssembly,
      .pViewportState = &viewportState,
      .pRasterizationState = &rasterizeState,
      .pMultisampleState = &multisampleState,
      .pDepthStencilState = &depthStencilState,
      .pColorBlendState = &blendState,
      .layout = pipelineLayout,
      .renderPass = VK_NULL_HANDLE,
  };
  VkFormat colorFormats[] = {this->surfaceFormat.format};
  VkPipelineRenderingCreateInfo renderingCI{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
      .colorAttachmentCount = 1,
      .pColorAttachmentFormats = colorFormats,
  };
  pipelineCreateInfo.pNext = &renderingCI;
  auto res = vkCreateGraphicsPipelines(
      device, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &this->pipeline);
  if (res != VK_SUCCESS) {
  }
  for (auto &m : shaderStages) {
    DestroyShaderModule(m.module);
  }
}
VkShaderModule VulkanRenderer::CreateShaderModule(const void *code,
                                                  size_t length) {
  VkShaderModuleCreateInfo shaderModuleCreateInfo{
      .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
      .codeSize = length,
      .pCode = reinterpret_cast<const uint32_t *>(code),
  };
  VkShaderModule shaderModule;
  vkCreateShaderModule(device, &shaderModuleCreateInfo, nullptr, &shaderModule);
  return shaderModule;
}
void VulkanRenderer::DestroyShaderModule(VkShaderModule shaderModule) {
  vkDestroyShaderModule(device, shaderModule, nullptr);
}
void VulkanRenderer::TransitionLayoutSwapchainImage(
    VkCommandBuffer commandBuffer, VkImageLayout newLayout,
    VkAccessFlags2 newAccessFlags) {
  auto &swapchainState = this->swapchainState[swapchainImageIndex];
  VkImageMemoryBarrier2 barrierToRT{
      .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
      .pNext = nullptr,
      .srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
      .srcAccessMask = swapchainState.accessFlags,
      .dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
      .dstAccessMask = newAccessFlags,
      .oldLayout = swapchainState.layout,
      .newLayout = newLayout,
      .image = swapchainState.image,
      .subresourceRange = {
          .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
          .baseMipLevel = 0,
          .levelCount = 1,
          .baseArrayLayer = 0,
          .layerCount = 1,
      }};

  VkDependencyInfo info{
      .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
      .dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT,
      .imageMemoryBarrierCount = 1,
      .pImageMemoryBarriers = &barrierToRT,
  };

  if (vkCmdPipelineBarrier2) {
    vkCmdPipelineBarrier2(commandBuffer, &info);
  } else {
    vkCmdPipelineBarrier2KHR(commandBuffer, &info);
  }
  swapchainState.layout = newLayout;
  swapchainState.accessFlags = newAccessFlags;
}
} // namespace paranoixa