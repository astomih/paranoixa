#include <webgpu/webgpu.h>
#ifdef EMSCRIPTEN
#include <emscripten.h>
#endif // EMSCRIPTEN
#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>

#include "webgpu_renderer.hpp"
#include <iostream>

namespace paranoixa {
WebGPURenderer::WebGPURenderer() {}
WebGPURenderer::~WebGPURenderer() {
  if (m_targetView) {
    wgpuTextureViewRelease(m_targetView);
  }
#ifndef __EMSCRIPTEN__
  if (m_surface) {
    wgpuSurfaceRelease(m_surface);
  }
#endif
  if (m_queue) {
    wgpuQueueRelease(m_queue);
  }
  if (m_device) {
    wgpuDeviceRelease(m_device);
  }
  if (m_adapter) {
    wgpuAdapterRelease(m_adapter);
  }
  if (m_instance) {
    wgpuInstanceRelease(m_instance);
  }
}
void WebGPURenderer::Initialize(void *window) {
  CreateInstance();
  CreateAdapter();
  CreateDevice();
  CreateQueue();
  CreateSurface(window);
  std::cout << "Surface created!" << std::endl;

  std::cout << "Surface Configuring... " << std::endl;
  ConfigSurface();
  std::cout << "Surface Configured!" << std::endl;
}
void WebGPURenderer::CreateSurface(void *window) {
#ifdef __EMSCRIPTEN__
  WGPUSurfaceDescriptorFromCanvasHTMLSelector canvasDesc = {};
  canvasDesc.chain.sType = WGPUSType_SurfaceDescriptorFromCanvasHTMLSelector;
  canvasDesc.selector = "#canvas";

  WGPUSurfaceDescriptor surfaceDesc = {};
  std::cout << "Creating surface..." << std::endl;
  surfaceDesc.nextInChain = reinterpret_cast<WGPUChainedStruct *>(&canvasDesc);

  m_surface = wgpuInstanceCreateSurface(m_instance, &surfaceDesc);
#endif
#ifdef WIN32
  {
    SDL_SysWMinfo windowWMInfo;
    SDL_VERSION(&windowWMInfo.version);
    SDL_GetWindowWMInfo((::SDL_Window *)window, &windowWMInfo);
    HWND hwnd = windowWMInfo.info.win.window;
    HINSTANCE hinstance = GetModuleHandle(NULL);
    WGPUSurfaceDescriptorFromWindowsHWND desc{
        .chain =
            (WGPUChainedStruct){
                .next = NULL,
                .sType = WGPUSType_SurfaceDescriptorFromWindowsHWND,
            },
        .hinstance = hinstance,
        .hwnd = hwnd};
    const WGPUChainedStruct *nextChain =
        reinterpret_cast<const WGPUChainedStruct *>(&desc);
    auto descriptor =
        (WGPUSurfaceDescriptor){.label = nullptr, .nextInChain = nextChain};
    m_surface = wgpuInstanceCreateSurface(m_instance, &descriptor);
  }
#endif
  std::cout << "Surface: " << m_surface << std::endl;
}
void WebGPURenderer::CreateInstance() {
#ifdef EMSCRIPTEN
  m_instance = wgpuCreateInstance(nullptr);
#else
  WGPUInstanceDescriptor desc = {};
  desc.nextInChain = nullptr;
  m_instance = wgpuCreateInstance(&desc);
#endif

  if (!m_instance) {
    std::cerr << "Could not initialize WebGPU!" << std::endl;
  }
  std::cout << "WGPU instance: " << m_instance << std::endl;
}
void WebGPURenderer::CreateAdapter() {
  struct UserData {
    WGPUAdapter adapter;
    bool adapterRequested;
  };
  auto onAdapterRequestEnded = [](WGPURequestAdapterStatus status,
                                  WGPUAdapter adapter, char const *message,
                                  void *pUserData) {
    if (status == WGPURequestAdapterStatus_Success) {
      UserData &userData = *reinterpret_cast<UserData *>(pUserData);
      userData.adapter = adapter;
      userData.adapterRequested = true;
    } else {
      std::cout << "Could not get WebGPU adapter: " << message << std::endl;
    }
  };
  UserData userData{};
  wgpuInstanceRequestAdapter(m_instance, nullptr, onAdapterRequestEnded,
                             &userData);
#ifdef __EMSCRIPTEN__
  while (!userData.adapterRequested) {
    emscripten_sleep(100);
  }
#endif // __EMSCRIPTEN__
  std::cout << "Adapter: " << userData.adapter << std::endl;
  m_adapter = userData.adapter;
}
void WebGPURenderer::CreateDevice() {
  struct UserData {
    WGPUDevice device = nullptr;
    bool requestEnded = false;
  };
  UserData userData;

  auto onDeviceRequestEnded = [](WGPURequestDeviceStatus status,
                                 WGPUDevice device, char const *message,
                                 void *pUserData) {
    UserData &userData = *reinterpret_cast<UserData *>(pUserData);
    if (status == WGPURequestDeviceStatus_Success) {
      userData.device = device;
    } else {
      std::cout << "Could not get WebGPU device: " << message << std::endl;
    }
    userData.requestEnded = true;
  };
  WGPUDeviceDescriptor descriptor{};
  wgpuAdapterRequestDevice(m_adapter, &descriptor, onDeviceRequestEnded,
                           (void *)&userData);

#ifdef __EMSCRIPTEN__
  while (!userData.requestEnded) {
    emscripten_sleep(100);
  }
#endif // __EMSCRIPTEN__
  std::cout << "Got device: " << userData.device << std::endl;
  m_device = userData.device;
}
void WebGPURenderer::CreateQueue() { m_queue = wgpuDeviceGetQueue(m_device); }
void WebGPURenderer::ConfigSurface() {
  if (!m_surface) {
    std::cout << "Surface is not created!" << std::endl;
    return;
  }
  if (!m_device) {
    std::cout << "Device is not created!" << std::endl;
    return;
  }
  if (!m_adapter) {
    std::cout << "Adapter is not created!" << std::endl;
    return;
  }
  WGPUSurfaceConfiguration config{};
  config.nextInChain = nullptr;
  config.width = 640;
  config.height = 480;
  config.usage = WGPUTextureUsage_RenderAttachment;
  WGPUTextureFormat surfaceFormat =
      wgpuSurfaceGetPreferredFormat(m_surface, m_adapter);
  config.format = surfaceFormat;
  config.viewFormatCount = 0;
  config.viewFormats = nullptr;
  config.device = m_device;
  config.presentMode = WGPUPresentMode_Fifo;
  config.alphaMode = WGPUCompositeAlphaMode_Auto;
  std::cout << "Surface configuring..." << std::endl;
  wgpuSurfaceConfigure(m_surface, &config);
  std::cout << "Surface configured!" << std::endl;
}
WGPUTextureView WebGPURenderer::GetNextSurfaceTextureView() {

  WGPUSurfaceTexture surfaceTexture;
  wgpuSurfaceGetCurrentTexture(m_surface, &surfaceTexture);
  if (surfaceTexture.status != WGPUSurfaceGetCurrentTextureStatus_Success) {
    return nullptr;
  }
  WGPUTextureViewDescriptor viewDescriptor;
  viewDescriptor.nextInChain = nullptr;
  viewDescriptor.label = "Surface texture view";
  viewDescriptor.format = wgpuTextureGetFormat(surfaceTexture.texture);
  viewDescriptor.dimension = WGPUTextureViewDimension_2D;
  viewDescriptor.baseMipLevel = 0;
  viewDescriptor.mipLevelCount = 1;
  viewDescriptor.baseArrayLayer = 0;
  viewDescriptor.arrayLayerCount = 1;
  viewDescriptor.aspect = WGPUTextureAspect_All;
  WGPUTextureView targetView =
      wgpuTextureCreateView(surfaceTexture.texture, &viewDescriptor);
  return targetView;
}
void WebGPURenderer::Render() {
  if (!m_surface) {
    std::cout << "Surface is not created!" << std::endl;
    return;
  }
  WGPUTextureView targetView = GetNextSurfaceTextureView();
  if (!targetView) {
    return;
  }

  // Create a command encoder
  WGPUCommandEncoderDescriptor encoderDesc{};
  encoderDesc.nextInChain = nullptr;
  WGPUCommandEncoder encoder =
      wgpuDeviceCreateCommandEncoder(m_device, &encoderDesc);

  // Create the render pass that clear the screen with our color
  WGPURenderPassColorAttachment colorAttachment{};
  colorAttachment.view = targetView;
  colorAttachment.resolveTarget = nullptr;
  colorAttachment.loadOp = WGPULoadOp_Clear;
  colorAttachment.storeOp = WGPUStoreOp_Store;
  colorAttachment.clearValue =
      (WGPUColor){.r = 0.2f, .g = 0.2f, .b = 1.0f, .a = 1.0f};

  WGPURenderPassDescriptor renderPassDesc{};
  renderPassDesc.nextInChain = nullptr;
  renderPassDesc.colorAttachmentCount = 1;
  renderPassDesc.colorAttachments = &colorAttachment;
  renderPassDesc.depthStencilAttachment = nullptr;
  WGPURenderPassEncoder renderPassEncoder =
      wgpuCommandEncoderBeginRenderPass(encoder, &renderPassDesc);
  wgpuRenderPassEncoderEnd(renderPassEncoder);
  wgpuRenderPassEncoderReference(renderPassEncoder);

  // Submit the command buffer
  WGPUCommandBufferDescriptor cmdBufferDesc{};
  cmdBufferDesc.nextInChain = nullptr;
  WGPUCommandBuffer cmdBuffer =
      wgpuCommandEncoderFinish(encoder, &cmdBufferDesc);
  wgpuCommandEncoderRelease(encoder);

  wgpuQueueSubmit(m_queue, 1, &cmdBuffer);
  wgpuCommandBufferRelease(cmdBuffer);
  wgpuTextureViewRelease(targetView);
  wgpuSurfacePresent(m_surface);
}
} // namespace paranoixa