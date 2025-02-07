#ifndef EMSCRIPTEN
#ifndef PARANOIXA_SDLGPU_RENDERER_HPP
#define PARANOIXA_SDLGPU_RENDERER_HPP
#include <paranoixa.hpp>

#include <SDL3/SDL_gpu.h>

#include <vector>

namespace paranoixa::sdlgpu {
namespace px = paranoixa;
class Device : public px::Device {
public:
  Device(const CreateInfo &createInfo, SDL_GPUDevice *device)
      : px::Device(createInfo), device(device), window(nullptr) {}
  SDL_GPUDevice *GetNative() { return device; }
  virtual ~Device() override;
  virtual void ClaimWindow(void *window) override;
  virtual Ptr<px::Buffer>
  CreateBuffer(const Buffer::CreateInfo &createInfo) override;
  virtual Ptr<px::Texture>
  CreateTexture(const Texture::CreateInfo &createInfo) override;
  virtual Ptr<px::Sampler>
  CreateSampler(const Sampler::CreateInfo &createInfo) override;
  virtual Ptr<px::TransferBuffer>
  CreateTransferBuffer(const TransferBuffer::CreateInfo &createInfo) override;
  virtual Ptr<px::Shader>
  CreateShader(const Shader::CreateInfo &createInfo) override;
  virtual Ptr<px::CommandBuffer>
  CreateCommandBuffer(const CommandBuffer::CreateInfo &createInfo) override;
  virtual Ptr<px::GraphicsPipeline> CreateGraphicsPipeline(
      const GraphicsPipeline::CreateInfo &createInfo) override;
  virtual Ptr<px::ComputePipeline>
  CreateComputePipeline(const ComputePipeline::CreateInfo &createInfo) override;
  virtual void
  SubmitCommandBuffer(Ptr<px::CommandBuffer> commandBuffer) override;
  virtual Ptr<px::Texture>
  AcquireSwapchainTexture(Ptr<px::CommandBuffer> commandBuffer) override;
  virtual String GetDriver() const override;

private:
  SDL_GPUDevice *device;
  SDL_Window *window;
};
class Texture : public px::Texture {
public:
  Texture(const CreateInfo &createInfo, Device &device, SDL_GPUTexture *texture,
          bool isSwapchainTexture = false)
      : px::Texture(createInfo), device(device), texture(texture),
        isSwapchainTexture(isSwapchainTexture) {}
  virtual ~Texture() override;

  inline SDL_GPUTexture *GetNative() const { return texture; }

private:
  Device &device;
  SDL_GPUTexture *texture;
  bool isSwapchainTexture;
};

class Sampler : public px::Sampler {
public:
  Sampler(const CreateInfo &createInfo, Device &device, SDL_GPUSampler *sampler)
      : px::Sampler(createInfo), device(device), sampler(sampler) {}
  ~Sampler() override;

  inline SDL_GPUSampler *GetNative() const { return sampler; }

private:
  Device &device;
  SDL_GPUSampler *sampler;
};

class TransferBuffer : public px::TransferBuffer {
public:
  TransferBuffer(const CreateInfo &createInfo, Device &device,
                 SDL_GPUTransferBuffer *transferBuffer)
      : px::TransferBuffer(createInfo), device(device),
        transferBuffer(transferBuffer) {}
  ~TransferBuffer() override;

  inline SDL_GPUTransferBuffer *GetNative() { return transferBuffer; }

  void *Map() override;
  void Unmap() override;

private:
  Device &device;
  SDL_GPUTransferBuffer *transferBuffer;
};
class Buffer : public px::Buffer {
public:
  Buffer(const CreateInfo &createInfo, Device &device, SDL_GPUBuffer *buffer)
      : px::Buffer(createInfo), device(device), buffer(buffer) {}
  ~Buffer() override;

  inline SDL_GPUBuffer *GetNative() { return buffer; }

private:
  Device &device;
  SDL_GPUBuffer *buffer;
};
class Backend : public px::Backend {
public:
  virtual Ptr<px::Device>
  CreateDevice(const px::Device::CreateInfo &createInfo) override;
};
class Shader : public px::Shader {
public:
  Shader(const CreateInfo &createInfo, Device &device, SDL_GPUShader *shader)
      : px::Shader(createInfo), device(device), shader(shader) {}
  ~Shader() override;

  inline SDL_GPUShader *GetNative() { return shader; }

private:
  Device &device;
  SDL_GPUShader *shader;
};

class CopyPass : public px::CopyPass {
public:
  CopyPass(AllocatorPtr allocator, class CommandBuffer &commandBuffer,
           SDL_GPUCopyPass *copyPass)
      : px::CopyPass(), commandBuffer(commandBuffer), copyPass(copyPass) {}
  inline SDL_GPUCopyPass *GetNative() { return copyPass; }

  virtual void UploadTexture(const TextureTransferInfo &src,
                             const TextureRegion &dst, bool cycle) override;
  virtual void DownloadTexture(const TextureRegion &src,
                               const TextureTransferInfo &dst) override;
  virtual void UploadBuffer(const BufferTransferInfo &src,
                            const BufferRegion &dst, bool cycle) override;
  virtual void DownloadBuffer(const BufferRegion &src,
                              const BufferTransferInfo &dst) override;

private:
  SDL_GPUCopyPass *copyPass;
  class CommandBuffer &commandBuffer;
};

class RenderPass : public px::RenderPass {
public:
  RenderPass(AllocatorPtr allocator, CommandBuffer &commandBuffer,
             SDL_GPURenderPass *renderPass)
      : px::RenderPass(), allocator(allocator), commandBuffer(commandBuffer),
        renderPass(renderPass) {}

  inline SDL_GPURenderPass *GetNative() const { return renderPass; }

  void BindGraphicsPipeline(Ptr<px::GraphicsPipeline> pipeline) override;
  void BindVertexBuffers(uint32 startSlot,
                         const Array<BufferBinding> &bindings) override;
  void
  BindFragmentSamplers(uint32 startSlot,
                       const Array<TextureSamplerBinding> &bindings) override;
  void DrawPrimitives(uint32 vertexCount, uint32 instanceCount,
                      uint32 firstVertex, uint32 firstInstance) override;

private:
  AllocatorPtr allocator;
  SDL_GPURenderPass *renderPass;
  class CommandBuffer &commandBuffer;
};

class CommandBuffer : public px::CommandBuffer {
public:
  CommandBuffer(const CreateInfo &createInfo,
                SDL_GPUCommandBuffer *commandBuffer)
      : px::CommandBuffer(createInfo), commandBuffer(commandBuffer) {}

  SDL_GPUCommandBuffer *GetNative() { return commandBuffer; }

  Ptr<px::CopyPass> BeginCopyPass() override;
  void EndCopyPass(Ptr<px::CopyPass> copyPass) override;
  Ptr<px::RenderPass>
  BeginRenderPass(const Array<px::ColorTargetInfo> &infos) override;
  void EndRenderPass(Ptr<px::RenderPass> renderPass) override;

private:
  SDL_GPUCommandBuffer *commandBuffer;
};

class GraphicsPipeline : public px::GraphicsPipeline {
public:
  GraphicsPipeline(const CreateInfo &createInfo, Device &device,
                   SDL_GPUGraphicsPipeline *pipeline)
      : px::GraphicsPipeline(createInfo), device(device), pipeline(pipeline) {}
  ~GraphicsPipeline() override;

  inline SDL_GPUGraphicsPipeline *GetNative() { return pipeline; }

private:
  Device &device;
  SDL_GPUGraphicsPipeline *pipeline;
};

class ComputePipeline : public px::ComputePipeline {
public:
  ComputePipeline(const CreateInfo &createInfo,
                  SDL_GPUComputePipeline *pipeline)
      : px::ComputePipeline(createInfo), pipeline(pipeline) {}

private:
  SDL_GPUComputePipeline *pipeline;
};

} // namespace paranoixa::sdlgpu
#endif // PARANOIXA_SDLGPU_RENDERER_HPP
#endif // EMSCRIPTEN