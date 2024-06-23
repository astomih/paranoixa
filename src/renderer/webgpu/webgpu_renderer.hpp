#ifndef PARANOIXA_WEBGPU_RENDERER_HPP
#define PARANOIXA_WEBGPU_RENDERER_HPP
#include "../renderer.hpp"

#include <SDL2/SDL.h>
#include <webgpu/webgpu.h>

namespace paranoixa {
class WebGPURenderer : public Renderer {
public:
  WebGPURenderer();
  ~WebGPURenderer();
  void Initialize(void *window) override;
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
  WGPUInstance m_instance;
  // WebGPU adapter
  WGPUAdapter m_adapter;
  // WebGPU device
  WGPUDevice m_device;
  // WebGPU queue
  WGPUQueue m_queue;
  // WebGPU surface
  WGPUSurface m_surface;
  // WebGPU surface texture view
  WGPUTextureView m_targetView;

  WGPUTextureFormat m_surfaceFormat;

  WGPURenderPipeline m_pipeline;
};
} // namespace paranoixa
#endif // PARANOIXA_WEBGPU_RENDERER_HPP