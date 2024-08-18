#ifndef PARANOIXA_WEBGPU_RENDERER_HPP
#define PARANOIXA_WEBGPU_RENDERER_HPP
#include "../renderer.hpp"

#include <SDL3/SDL.h>
#include <webgpu/webgpu.h>

namespace paranoixa {
class WebGPURenderer : public Renderer {
public:
  WebGPURenderer();
  ~WebGPURenderer();
  void Initialize(void *window) override;
  void ProcessEvent(void *event) override;
  void Render() override;

private:
  void CreateSurface(void *window);
  void CreateInstance();
  void CreateAdapter();
  void CreateDevice();
  void CreateQueue();

  void ConfigSurface();

  void InitializePipeline();

  WGPUTextureView GetNextSurfaceTextureView();

  // WebGPU instance
  WGPUInstance instance;
  // WebGPU adapter
  WGPUAdapter adapter;
  // WebGPU device
  WGPUDevice device;
  // WebGPU queue
  WGPUQueue queue;
  // WebGPU surface
  WGPUSurface surface;
  // WebGPU surface texture view
  WGPUTextureView targetView;

  WGPUTextureFormat surfaceFormat;

  WGPURenderPipeline pipeline;
};
} // namespace paranoixa
#endif // PARANOIXA_WEBGPU_RENDERER_HPP