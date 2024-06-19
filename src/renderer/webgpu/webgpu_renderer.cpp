#include <webgpu/webgpu.h>
#ifdef EMSCRIPTEN
#include <emscripten.h>
#include <emscripten/html5.h>
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
  ConfigSurface();
  InitializePipeline();
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

  m_surface = wgpuInstanceCreateSurface(m_instance, &surfaceDescriptor);
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
    auto descriptor =
        (WGPUSurfaceDescriptor){.nextInChain = &desc.chain, .label = NULL};
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
  descriptor.deviceLostCallback = [](WGPUDeviceLostReason reason,
                                     char const *message,
                                     void * /* pUserData */) {
    std::cout << "Device lost: reason " << reason;
    if (message)
      std::cout << " (" << message << ")";
    std::cout << std::endl;
  };

  wgpuAdapterRequestDevice(m_adapter, &descriptor, onDeviceRequestEnded,
                           (void *)&userData);

#ifdef __EMSCRIPTEN__
  while (!userData.requestEnded) {
    emscripten_sleep(100);
  }
#endif // __EMSCRIPTEN__
  std::cout << "Device: " << userData.device << std::endl;
  m_device = userData.device;
  auto onDeviceError = [](WGPUErrorType type, char const *message,
                          void * /* pUserData */) {
    std::cout << "Uncaptured device error: type " << type;
    if (message)
      std::cout << " (" << message << ")";
    std::cout << std::endl;
  };
  wgpuDeviceSetUncapturedErrorCallback(m_device, onDeviceError,
                                       nullptr /* pUserData */);
}
void WebGPURenderer::CreateQueue() { m_queue = wgpuDeviceGetQueue(m_device); }
void WebGPURenderer::ConfigSurface() {
  WGPUSurfaceConfiguration config = {};
  config.nextInChain = nullptr;

  config.width = 640;
  config.height = 480;
  config.usage = WGPUTextureUsage_RenderAttachment;
  m_surfaceFormat = wgpuSurfaceGetPreferredFormat(m_surface, m_adapter);
  std::cout << "Surface format: " << m_surfaceFormat << std::endl;
  config.format = m_surfaceFormat;
  config.viewFormatCount = 0;
  config.viewFormats = nullptr;
  config.device = m_device;
  config.presentMode = WGPUPresentMode_Fifo;
  config.alphaMode = WGPUCompositeAlphaMode_Auto;
  wgpuSurfaceConfigure(m_surface, &config);
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
    std::cout << "Could not get target view!" << std::endl;
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
      WGPUColor{.r = 1.0f, .g = 0.4f, .b = 0.0f, .a = 1.0f};
#ifndef WIN32
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
  wgpuRenderPassEncoderSetPipeline(renderPass, m_pipeline);
  wgpuRenderPassEncoderDraw(renderPass, 3, 1, 0, 0);

  wgpuRenderPassEncoderEnd(renderPass);
  wgpuRenderPassEncoderReference(renderPass);

  // Submit the command buffer
  WGPUCommandBufferDescriptor cmdBufferDesc{};
  cmdBufferDesc.nextInChain = nullptr;
  WGPUCommandBuffer cmdBuffer =
      wgpuCommandEncoderFinish(encoder, &cmdBufferDesc);
  wgpuCommandEncoderRelease(encoder);

  wgpuQueueSubmit(m_queue, 1, &cmdBuffer);
  wgpuCommandBufferRelease(cmdBuffer);
  wgpuTextureViewRelease(targetView);
#ifndef __EMSCRIPTEN__
  wgpuSurfacePresent(m_surface);
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
      wgpuDeviceCreateShaderModule(m_device, &shaderDesc);

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
  colorTarget.format = m_surfaceFormat;
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

  m_pipeline = wgpuDeviceCreateRenderPipeline(m_device, &pipelineDesc);

  // We no longer need to access the shader module
  wgpuShaderModuleRelease(shaderModule);
}
} // namespace paranoixa