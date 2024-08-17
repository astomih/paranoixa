#include <webgpu/webgpu.h>
#ifdef EMSCRIPTEN
#include <emscripten.h>
#include <emscripten/html5.h>
#endif // EMSCRIPTEN
#define SDL_MAIN_HANDLED
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_system.h>

#include <imgui.h>

#include <backends/imgui_impl_sdl3.h>
#include <backends/imgui_impl_wgpu.h>

#ifdef _WIN32
#include <windows.h>
#endif

#include "webgpu_renderer.hpp"
#include <iostream>

namespace paranoixa {
WebGPURenderer::WebGPURenderer() {}
WebGPURenderer::~WebGPURenderer() {
#ifndef __EMSCRIPTEN__
  if (surface) {
    wgpuSurfaceRelease(surface);
  }
#endif
  if (queue) {
    wgpuQueueRelease(queue);
  }
  if (device) {
    wgpuDeviceRelease(device);
  }
  if (adapter) {
    wgpuAdapterRelease(adapter);
  }
  if (instance) {
    wgpuInstanceRelease(instance);
  }
}
void WebGPURenderer::Initialize(void *window) {
  CreateInstance();
  CreateAdapter();
  CreateDevice();
  CreateQueue();
  CreateSurface(window);
  ConfigSurface();
  InitializePipeline();
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  (void)io;
  ImGui::StyleColorsDark();
  ImGui_ImplSDL3_InitForOther((SDL_Window *)window);
  ImGui_ImplWGPU_InitInfo init_info{};
  init_info.Device = device;
  init_info.RenderTargetFormat = surfaceFormat;
  ImGui_ImplWGPU_Init(&init_info);

  std::cout << "WebGPU renderer initialized!" << std::endl;
}
void WebGPURenderer::CreateSurface(void *window) {
#ifdef __EMSCRIPTEN__
  WGPUSurfaceDescriptorFromCanvasHTMLSelector fromCanvasHTMLSelector = {};
  fromCanvasHTMLSelector.chain.next = NULL;
  fromCanvasHTMLSelector.chain.sType =
      WGPUSType_SurfaceDescriptorFromCanvasHTMLSelector;
  fromCanvasHTMLSelector.selector = "canvas";

  WGPUSurfaceDescriptor surfaceDescriptor = {};
  surfaceDescriptor.nextInChain = &fromCanvasHTMLSelector.chain;
  surfaceDescriptor.label = NULL;

  surface = wgpuInstanceCreateSurface(instance, &surfaceDescriptor);
#endif
#ifdef _WIN32
  {
    HWND hwnd = (HWND)SDL_GetPointerProperty(
        SDL_GetWindowProperties((::SDL_Window *)window),
        SDL_PROP_WINDOW_WIN32_HWND_POINTER, NULL);
    HINSTANCE hinstance = GetModuleHandle(NULL);
    WGPUSurfaceDescriptorFromWindowsHWND desc{
        .chain =
            WGPUChainedStruct{
                .next = NULL,
                .sType = WGPUSType_SurfaceDescriptorFromWindowsHWND,
            },
        .hinstance = hinstance,
        .hwnd = hwnd};
    auto descriptor =
        WGPUSurfaceDescriptor{.nextInChain = &desc.chain, .label = NULL};
    surface = wgpuInstanceCreateSurface(instance, &descriptor);
  }
#endif
  std::cout << "Surface: " << surface << std::endl;
}
void WebGPURenderer::CreateInstance() {
#ifdef EMSCRIPTEN
  instance = wgpuCreateInstance(nullptr);
#else
  WGPUInstanceDescriptor desc = {};
  desc.nextInChain = nullptr;
  instance = wgpuCreateInstance(&desc);
#endif

  if (!instance) {
    std::cerr << "Could not initialize WebGPU!" << std::endl;
  }
  std::cout << "WGPU instance: " << instance << std::endl;
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
  wgpuInstanceRequestAdapter(instance, nullptr, onAdapterRequestEnded,
                             &userData);
#ifdef __EMSCRIPTEN__
  while (!userData.adapterRequested) {
    emscripten_sleep(100);
  }
#endif // __EMSCRIPTEN__
  std::cout << "Adapter: " << userData.adapter << std::endl;
  adapter = userData.adapter;
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
  descriptor.deviceLostCallback = [](WGPUDeviceLostReason reason,
                                     char const *message,
                                     void * /* pUserData */) {
    std::cout << "Device lost: reason " << reason;
    if (message)
      std::cout << " (" << message << ")";
    std::cout << std::endl;
  };

  wgpuAdapterRequestDevice(adapter, &descriptor, onDeviceRequestEnded,
                           (void *)&userData);

#ifdef __EMSCRIPTEN__
  while (!userData.requestEnded) {
    emscripten_sleep(100);
  }
#endif // __EMSCRIPTEN__
  std::cout << "Device: " << userData.device << std::endl;
  device = userData.device;
  auto onDeviceError = [](WGPUErrorType type, char const *message,
                          void * /* pUserData */) {
    std::cout << "Uncaptured device error: type " << type;
    if (message)
      std::cout << " (" << message << ")";
    std::cout << std::endl;
  };
  wgpuDeviceSetUncapturedErrorCallback(device, onDeviceError,
                                       nullptr /* pUserData */);
}
void WebGPURenderer::CreateQueue() { queue = wgpuDeviceGetQueue(device); }
void WebGPURenderer::ConfigSurface() {
  WGPUSurfaceConfiguration config = {};
  config.nextInChain = nullptr;

  config.width = 640;
  config.height = 480;
  config.usage = WGPUTextureUsage_RenderAttachment;
  surfaceFormat = WGPUTextureFormat_BGRA8UnormSrgb;
  surfaceFormat = wgpuSurfaceGetPreferredFormat(surface, adapter);
  surfaceFormat = WGPUTextureFormat_RGBA8Unorm;
  std::cout << "Surface format: " << surfaceFormat << std::endl;
  config.format = surfaceFormat;
  config.viewFormatCount = 0;
  config.viewFormats = nullptr;
  config.device = device;
  config.presentMode = WGPUPresentMode_Fifo;
  config.alphaMode = WGPUCompositeAlphaMode_Auto;
  wgpuSurfaceConfigure(surface, &config);
}
WGPUTextureView WebGPURenderer::GetNextSurfaceTextureView() {

  WGPUSurfaceTexture surfaceTexture;
  wgpuSurfaceGetCurrentTexture(surface, &surfaceTexture);
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
  if (!surface) {
    std::cout << "Surface is not created!" << std::endl;
    return;
  }
  WGPUTextureView targetView = GetNextSurfaceTextureView();
  if (!targetView) {
    std::cout << "Could not get target view!" << std::endl;
    return;
  }

  // Create a command encoder
  WGPUCommandEncoderDescriptor encoderDesc{};
  encoderDesc.nextInChain = nullptr;
  WGPUCommandEncoder encoder =
      wgpuDeviceCreateCommandEncoder(device, &encoderDesc);

  // Create the render pass that clear the screen with our color
  WGPURenderPassColorAttachment colorAttachment{};
  colorAttachment.view = targetView;
  colorAttachment.resolveTarget = nullptr;
  colorAttachment.loadOp = WGPULoadOp_Clear;
  colorAttachment.storeOp = WGPUStoreOp_Store;
  colorAttachment.clearValue =
      WGPUColor{.r = 1.0f, .g = 0.4f, .b = 0.0f, .a = 1.0f};
#ifndef _WIN32
  colorAttachment.depthSlice = WGPU_DEPTH_SLICE_UNDEFINED;
#endif

  WGPURenderPassDescriptor renderPassDesc{};
  renderPassDesc.nextInChain = nullptr;
  renderPassDesc.colorAttachmentCount = 1;
  renderPassDesc.colorAttachments = &colorAttachment;
  renderPassDesc.depthStencilAttachment = nullptr;
  WGPURenderPassEncoder renderPass =
      wgpuCommandEncoderBeginRenderPass(encoder, &renderPassDesc);

  // Set the pipeline and draw
  wgpuRenderPassEncoderSetPipeline(renderPass, pipeline);
  wgpuRenderPassEncoderDraw(renderPass, 3, 1, 0, 0);

  ImGui_ImplWGPU_NewFrame();
  ImGui_ImplSDL3_NewFrame();
  ImGui::NewFrame();
  ImGui::ShowDemoWindow();
  ImGui::Begin("Hello, world!");
  ImGui::End();
  ImGui::Render();
  ImGui_ImplWGPU_RenderDrawData(ImGui::GetDrawData(), renderPass);
  wgpuRenderPassEncoderEnd(renderPass);
  wgpuRenderPassEncoderReference(renderPass);

  // Submit the command buffer
  WGPUCommandBufferDescriptor cmdBufferDesc{};
  cmdBufferDesc.nextInChain = nullptr;
  WGPUCommandBuffer cmdBuffer =
      wgpuCommandEncoderFinish(encoder, &cmdBufferDesc);
  wgpuCommandEncoderRelease(encoder);

  wgpuQueueSubmit(queue, 1, &cmdBuffer);
  wgpuCommandBufferRelease(cmdBuffer);
  wgpuTextureViewRelease(targetView);
#ifndef __EMSCRIPTEN__
  wgpuSurfacePresent(surface);
#endif
}
void WebGPURenderer::InitializePipeline() {
  WGPUShaderModuleDescriptor shaderDesc{};
#ifndef __EMSCRIPTEN__
  shaderDesc.hintCount = 0;
  shaderDesc.hints = nullptr;
#endif
  WGPUShaderModuleWGSLDescriptor wgslDesc{};
  wgslDesc.chain.next = nullptr;
  wgslDesc.chain.sType = WGPUSType_ShaderModuleWGSLDescriptor;
  shaderDesc.nextInChain = &wgslDesc.chain;
  wgslDesc.code = R"(
@vertex
fn vs_main(@builtin(vertex_index) in_vertex_index: u32) -> @builtin(position) vec4f {
	var p = vec2f(0.0, 0.0);
	if (in_vertex_index == 0u) {
		p = vec2f(-0.5, -0.5);
	} else if (in_vertex_index == 1u) {
		p = vec2f(0.5, -0.5);
	} else {
		p = vec2f(0.0, 0.5);
	}
	return vec4f(p, 0.0, 1.0);
}

@fragment
fn fs_main() -> @location(0) vec4f {
	return vec4f(1.0, 1.0, 1.0, 1.0);
}
)";
  WGPUShaderModule shaderModule =
      wgpuDeviceCreateShaderModule(device, &shaderDesc);

  WGPURenderPipelineDescriptor pipelineDesc{};
  pipelineDesc.nextInChain = nullptr;
  pipelineDesc.vertex.bufferCount = 0;
  pipelineDesc.vertex.buffers = nullptr;

  pipelineDesc.vertex.module = shaderModule;
  pipelineDesc.vertex.entryPoint = "vs_main";
  pipelineDesc.vertex.constantCount = 0;
  pipelineDesc.vertex.constants = nullptr;

  pipelineDesc.primitive.topology = WGPUPrimitiveTopology_TriangleList;
  pipelineDesc.primitive.stripIndexFormat = WGPUIndexFormat_Undefined;
  pipelineDesc.primitive.frontFace = WGPUFrontFace_CCW;
  pipelineDesc.primitive.cullMode = WGPUCullMode_None;

  WGPUFragmentState fragmentState{};
  fragmentState.module = shaderModule;
  fragmentState.entryPoint = "fs_main";
  fragmentState.constantCount = 0;
  fragmentState.constants = nullptr;

  WGPUBlendState blendState{};
  blendState.color.srcFactor = WGPUBlendFactor_SrcAlpha;
  blendState.color.dstFactor = WGPUBlendFactor_OneMinusSrcAlpha;
  blendState.color.operation = WGPUBlendOperation_Add;
  blendState.alpha.srcFactor = WGPUBlendFactor_Zero;
  blendState.alpha.dstFactor = WGPUBlendFactor_One;
  blendState.alpha.operation = WGPUBlendOperation_Add;

  WGPUColorTargetState colorTarget{};
  colorTarget.format = surfaceFormat;
  colorTarget.blend = &blendState;
  colorTarget.writeMask = WGPUColorWriteMask_All; // We could write to only some
                                                  // of the color channels.

  // We have only one target because our render pass has only one output color
  // attachment.
  fragmentState.targetCount = 1;
  fragmentState.targets = &colorTarget;
  pipelineDesc.fragment = &fragmentState;

  // We do not use stencil/depth testing for now
  pipelineDesc.depthStencil = nullptr;

  // Samples per pixel
  pipelineDesc.multisample.count = 1;

  // Default value for the mask, meaning "all bits on"
  pipelineDesc.multisample.mask = ~0u;

  // Default value as well (irrelevant for count = 1 anyways)
  pipelineDesc.multisample.alphaToCoverageEnabled = false;

  pipelineDesc.layout = nullptr;

  pipeline = wgpuDeviceCreateRenderPipeline(device, &pipelineDesc);

  // We no longer need to access the shader module
  wgpuShaderModuleRelease(shaderModule);
}
} // namespace paranoixa