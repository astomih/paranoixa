#ifndef EMSCRIPTEN
#ifndef PARANOIXA_SDLGPU_RENDERER_HPP
#define PARANOIXA_SDLGPU_RENDERER_HPP
#include "paranoixa.hpp"
#include "renderer/renderer.hpp"

#include <SDL3/SDL_gpu.h>

namespace paranoixa {
class SDLGPUDevice : public Device {
public:
  SDLGPUDevice(const CreateInfo &createInfo, SDL_GPUDevice *device)
      : Device(createInfo), device(device) {}
  SDL_GPUDevice *GetNative() { return device; }
  virtual ~SDLGPUDevice() override {}
  virtual void ClaimWindow(void *window) override;
  virtual Ptr<Buffer>
  CreateBuffer(const Buffer::CreateInfo &createInfo) override;
  virtual Ptr<Texture>
  CreateTexture(const Texture::CreateInfo &createInfo) override;
  virtual Ptr<Sampler>
  CreateSampler(const Sampler::CreateInfo &createInfo) override;
  virtual Ptr<TransferBuffer>
  CreateTransferBuffer(const TransferBuffer::CreateInfo &createInfo) override;
  virtual Ptr<Shader>
  CreateShader(const Shader::CreateInfo &createInfo) override;
  virtual Ptr<CommandBuffer>
  CreateCommandBuffer(const CommandBuffer::CreateInfo &createInfo) override;
  virtual Ptr<GraphicsPipeline> CreateGraphicsPipeline(
      const GraphicsPipeline::CreateInfo &createInfo) override;
  virtual Ptr<ComputePipeline>
  CreateComputePipeline(const ComputePipeline::CreateInfo &createInfo) override;

private:
  SDL_GPUDevice *device;
};
class SDLGPUTexture : public Texture {
public:
  SDLGPUTexture(const CreateInfo &createInfo, Device &device,
                SDL_GPUTexture *texture)
      : Texture(createInfo), texture(texture) {}

private:
  SDL_GPUTexture *texture;
};

class SDLGPUSampler : public Sampler {
public:
  SDLGPUSampler(const CreateInfo &createInfo, SDL_GPUSampler *sampler)
      : Sampler(createInfo), sampler(sampler) {}

private:
  SDL_GPUSampler *sampler;
};

class SDLGPUTransferBuffer : public TransferBuffer {
public:
  SDLGPUTransferBuffer(const CreateInfo &createInfo, SDLGPUDevice &device,
                       SDL_GPUTransferBuffer *transferBuffer)
      : TransferBuffer(createInfo), device(device),
        transferBuffer(transferBuffer) {}

  void *Map() override;
  void Unmap() override;

private:
  SDLGPUDevice &device;
  SDL_GPUTransferBuffer *transferBuffer;
};
class SDLGPUBuffer : public Buffer {
public:
  SDLGPUBuffer(const CreateInfo &createInfo, SDL_GPUBuffer *buffer)
      : Buffer(createInfo), buffer(buffer) {}

private:
  SDL_GPUBuffer *buffer;
};
class SDLGPUBackend : public Backend {
public:
  virtual Ptr<Device>
  CreateDevice(const Device::CreateInfo &createInfo) override;
};
class SDLGPUShader : public Shader {
public:
  SDLGPUShader(const CreateInfo &createInfo, SDL_GPUShader *shader)
      : Shader(createInfo), shader(shader) {}

private:
  SDL_GPUShader *shader;
};

class SDLGPUCommandBuffer : public CommandBuffer {
public:
  SDLGPUCommandBuffer(const CreateInfo &createInfo,
                      SDL_GPUCommandBuffer *commandBuffer)
      : CommandBuffer(createInfo), commandBuffer(commandBuffer) {}

private:
  SDL_GPUCommandBuffer *commandBuffer;
};

class SDLGPUGraphicsPipeline : public GraphicsPipeline {
public:
  SDLGPUGraphicsPipeline(const CreateInfo &createInfo,
                         SDL_GPUGraphicsPipeline *pipeline)
      : GraphicsPipeline(createInfo), pipeline(pipeline) {}

private:
  SDL_GPUGraphicsPipeline *pipeline;
};

class SDLGPUComputePipeline : public ComputePipeline {
public:
  SDLGPUComputePipeline(const CreateInfo &createInfo,
                        SDL_GPUComputePipeline *pipeline)
      : ComputePipeline(createInfo), pipeline(pipeline) {}

private:
  SDL_GPUComputePipeline *pipeline;
};

class SDLGPURenderer : public Renderer {

public:
  SDLGPURenderer(AllocatorPtr allcator);
  ~SDLGPURenderer() override;

  void Initialize(void *window) override;
  void ProcessEvent(void *event) override;
  void BeginFrame() override;
  void EndFrame() override;
  void AddGuiUpdateCallBack(std::function<void()> callBack) override;
};
} // namespace paranoixa
#endif // PARANOIXA_SDLGPU_RENDERER_HPP
#endif // EMSCRIPTEN