#ifndef EMSCRIPTEN
#ifndef PARANOIXA_SDLGPU_RENDERER_HPP
#define PARANOIXA_SDLGPU_RENDERER_HPP
#include <paranoixa.hpp>

#include <SDL3/SDL_gpu.h>

#include <vector>

namespace paranoixa {
class SDLGPUDevice : public Device {
public:
  SDLGPUDevice(const CreateInfo &createInfo, SDL_GPUDevice *device)
      : Device(createInfo), device(device), window(nullptr) {}
  SDL_GPUDevice *GetNative() { return device; }
  virtual ~SDLGPUDevice() override;
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
  virtual void SubmitCommandBuffer(Ptr<CommandBuffer> commandBuffer) override;
  virtual Ptr<Texture>
  AcquireSwapchainTexture(Ptr<CommandBuffer> commandBuffer) override;

private:
  SDL_GPUDevice *device;
  SDL_Window *window;
};
class SDLGPUTexture : public Texture {
public:
  SDLGPUTexture(const CreateInfo &createInfo, SDLGPUDevice &device,
                SDL_GPUTexture *texture, bool isSwapchainTexture = false)
      : Texture(createInfo), device(device), texture(texture),
        isSwapchainTexture(isSwapchainTexture) {}
  virtual ~SDLGPUTexture() override;

  inline SDL_GPUTexture *GetNative() const { return texture; }

private:
  SDLGPUDevice &device;
  SDL_GPUTexture *texture;
  bool isSwapchainTexture;
};

class SDLGPUSampler : public Sampler {
public:
  SDLGPUSampler(const CreateInfo &createInfo, SDLGPUDevice &device,
                SDL_GPUSampler *sampler)
      : Sampler(createInfo), device(device), sampler(sampler) {}
  ~SDLGPUSampler() override;

  inline SDL_GPUSampler *GetNative() const { return sampler; }

private:
  SDLGPUDevice &device;
  SDL_GPUSampler *sampler;
};

class SDLGPUTransferBuffer : public TransferBuffer {
public:
  SDLGPUTransferBuffer(const CreateInfo &createInfo, SDLGPUDevice &device,
                       SDL_GPUTransferBuffer *transferBuffer)
      : TransferBuffer(createInfo), device(device),
        transferBuffer(transferBuffer) {}
  ~SDLGPUTransferBuffer() override;

  inline SDL_GPUTransferBuffer *GetNative() { return transferBuffer; }

  void *Map() override;
  void Unmap() override;

private:
  SDLGPUDevice &device;
  SDL_GPUTransferBuffer *transferBuffer;
};
class SDLGPUBuffer : public Buffer {
public:
  SDLGPUBuffer(const CreateInfo &createInfo, SDLGPUDevice &device,
               SDL_GPUBuffer *buffer)
      : Buffer(createInfo), device(device), buffer(buffer) {}
  ~SDLGPUBuffer() override;

  inline SDL_GPUBuffer *GetNative() { return buffer; }

private:
  SDLGPUDevice &device;
  SDL_GPUBuffer *buffer;
};
class SDLGPUBackend : public Backend {
public:
  virtual Ptr<Device>
  CreateDevice(const Device::CreateInfo &createInfo) override;
};
class SDLGPUShader : public Shader {
public:
  SDLGPUShader(const CreateInfo &createInfo, SDLGPUDevice &device,
               SDL_GPUShader *shader)
      : Shader(createInfo), device(device), shader(shader) {}
  ~SDLGPUShader() override;

  inline SDL_GPUShader *GetNative() { return shader; }

private:
  SDLGPUDevice &device;
  SDL_GPUShader *shader;
};

class SDLGPUCopyPass : public CopyPass {
public:
  SDLGPUCopyPass(AllocatorPtr allocator,
                 class SDLGPUCommandBuffer &commandBuffer,
                 SDL_GPUCopyPass *copyPass)
      : CopyPass(), commandBuffer(commandBuffer), copyPass(copyPass) {}
  inline SDL_GPUCopyPass *GetNative() { return copyPass; }

  virtual void UploadTexture(const TextureTransferInfo &src,
                             const TextureRegion &dst, bool cycle) override;
  virtual void DownloadTexture(const TextureRegion &src,
                               const TextureTransferInfo &dst,
                               bool cycle) override;
  virtual void UploadBuffer(const BufferTransferInfo &src,
                            const BufferRegion &dst, bool cycle) override;
  virtual void DownloadBuffer(const BufferRegion &src,
                              const BufferTransferInfo &dst,
                              bool cycle) override;

private:
  SDL_GPUCopyPass *copyPass;
  class SDLGPUCommandBuffer &commandBuffer;
};

class SDLGPURenderPass : public RenderPass {
public:
  SDLGPURenderPass(AllocatorPtr allocator, SDLGPUCommandBuffer &commandBuffer,
                   SDL_GPURenderPass *renderPass)
      : RenderPass(), allocator(allocator), commandBuffer(commandBuffer),
        renderPass(renderPass) {}

  inline SDL_GPURenderPass *GetNative() const { return renderPass; }

  void BindGraphicsPipeline(Ptr<GraphicsPipeline> pipeline) override;
  void BindVertexBuffers(uint32_t startSlot,
                         const Array<BufferBinding> &bindings) override;
  void
  BindFragmentSamplers(uint32_t startSlot,
                       const Array<TextureSamplerBinding> &bindings) override;
  void DrawPrimitives(uint32_t vertexCount, uint32_t instanceCount,
                      uint32_t firstVertex, uint32_t firstInstance) override;

private:
  AllocatorPtr allocator;
  SDL_GPURenderPass *renderPass;
  class SDLGPUCommandBuffer &commandBuffer;
};

class SDLGPUCommandBuffer : public CommandBuffer {
public:
  SDLGPUCommandBuffer(const CreateInfo &createInfo,
                      SDL_GPUCommandBuffer *commandBuffer)
      : CommandBuffer(createInfo), commandBuffer(commandBuffer) {}

  SDL_GPUCommandBuffer *GetNative() { return commandBuffer; }

  Ptr<CopyPass> BeginCopyPass() override;
  void EndCopyPass(Ptr<CopyPass> copyPass) override;
  Ptr<RenderPass>
  BeginRenderPass(const Array<RenderPass::ColorTargetInfo> &infos) override;
  void EndRenderPass(Ptr<RenderPass> renderPass) override;

private:
  SDL_GPUCommandBuffer *commandBuffer;
};

class SDLGPUGraphicsPipeline : public GraphicsPipeline {
public:
  SDLGPUGraphicsPipeline(const CreateInfo &createInfo, SDLGPUDevice &device,
                         SDL_GPUGraphicsPipeline *pipeline)
      : GraphicsPipeline(createInfo), device(device), pipeline(pipeline) {}
  ~SDLGPUGraphicsPipeline() override;

  inline SDL_GPUGraphicsPipeline *GetNative() { return pipeline; }

private:
  SDLGPUDevice &device;
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

} // namespace paranoixa
#endif // PARANOIXA_SDLGPU_RENDERER_HPP
#endif // EMSCRIPTEN