#ifndef PARANOIXA_WEBGPU_RENDERER_HPP
#define PARANOIXA_WEBGPU_RENDERER_HPP
#include <renderer/renderer.hpp>

#include <SDL3/SDL.h>
#include <webgpu/webgpu.h>

namespace paranoixa {
class WebGPURenderer : public Renderer {
public:
  WebGPURenderer(AllocatorPtr pAllocator);
  ~WebGPURenderer() override;
  void Initialize(void *window) override;
  void ProcessEvent(void *event) override;
  void BeginFrame() override;
  void EndFrame() override;

  void AddGuiUpdateCallBack(std::function<void()> callBack) override;
  class Texture {
  public:
    Texture() = default;
    ~Texture() = default;
    WGPUTexture texture;
    WGPUTextureView view;
  };

private:
  void PrepareSurface(void *window);
  void PrepareInstance();
  void PrepareAdapter();
  void PrepareDevice();
  void PrepareQueue();

  void ConfigSurface(uint32_t width, uint32_t height);
  Texture CreateTexture(const void *data, size_t size, int width, int height);
  WGPUBuffer CreateBuffer(uint64_t size, WGPUBufferUsage usage);
  void PrepareSampler();
  void InitializePipeline();

  WGPUTextureView GetNextSurfaceTextureView();
  WGPUStringView GetStringView(const char *str);

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
  WGPUBindGroup bindGroup;

  WGPUTextureFormat surfaceFormat;
  Texture texture;
  WGPUSampler sampler;
  WGPURenderPipeline pipeline;
  WGPUBuffer vertexBuffer;
  WGPUCommandEncoder encoder;
};
} // namespace paranoixa
#endif // PARANOIXA_WEBGPU_RENDERER_HPP