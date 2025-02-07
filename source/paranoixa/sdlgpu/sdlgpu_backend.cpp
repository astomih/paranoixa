#ifndef EMSCRIPTEN
#include "sdlgpu_backend.hpp"
#include "sdlgpu_convert.hpp"

#include <SDL3/SDL.h>
#include <SDL3/SDL_gpu.h>

namespace paranoixa::sdlgpu {
Ptr<px::Device> Backend::CreateDevice(const Device::CreateInfo &createInfo) {
  SDL_GPUDevice *device = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV,
                                              createInfo.debugMode, nullptr);
  if (!device) {
    std::cout << "Failed to create GPU device" << std::endl;
  }
  return MakePtr<Device>(createInfo.allocator, createInfo, device);
}
void CopyPass::UploadTexture(const TextureTransferInfo &src,
                             const TextureRegion &dst, bool cycle) {
  SDL_GPUTextureTransferInfo transferInfo = {
      .transfer_buffer =
          DownCast<TransferBuffer>(src.transferBuffer)->GetNative(),
      .offset = src.offset,
  };
  SDL_GPUTextureRegion region = {
      .texture = DownCast<Texture>(dst.texture)->GetNative(),
      .x = dst.x,
      .y = dst.y,
      .z = dst.z,
      .w = dst.width,
      .h = dst.height,
      .d = dst.depth,
  };
  SDL_UploadToGPUTexture(this->copyPass, &transferInfo, &region, cycle);
}
void CopyPass::DownloadTexture(const TextureRegion &src,
                               const TextureTransferInfo &dst) {

  SDL_GPUTextureTransferInfo transferInfo = {
      .transfer_buffer =
          DownCast<TransferBuffer>(dst.transferBuffer)->GetNative(),
      .offset = dst.offset,
  };
  SDL_GPUTextureRegion region = {

      .texture = DownCast<Texture>(src.texture)->GetNative(),
      .x = src.x,
      .y = src.y,
      .z = src.z,
      .w = src.width,
      .h = src.height,
      .d = src.depth,
  };
  SDL_DownloadFromGPUTexture(this->copyPass, &region, &transferInfo);
}
void CopyPass::UploadBuffer(const BufferTransferInfo &src,
                            const BufferRegion &dst, bool cycle) {
  SDL_GPUTransferBufferLocation transferInfo = {
      .transfer_buffer =
          DownCast<TransferBuffer>(src.transferBuffer)->GetNative(),
      .offset = src.offset};
  SDL_GPUBufferRegion region = {.buffer =
                                    DownCast<Buffer>(dst.buffer)->GetNative(),
                                .offset = dst.offset,
                                .size = dst.size};
  SDL_UploadToGPUBuffer(this->copyPass, &transferInfo, &region, cycle);
}
void CopyPass::DownloadBuffer(const BufferRegion &src,
                              const BufferTransferInfo &dst) {
  SDL_GPUBufferRegion region = {.buffer =
                                    DownCast<Buffer>(src.buffer)->GetNative(),
                                .offset = src.offset,
                                .size = src.size};
  SDL_GPUTransferBufferLocation transferInfo = {
      .transfer_buffer =
          DownCast<TransferBuffer>(dst.transferBuffer)->GetNative(),
      .offset = dst.offset,
  };
  SDL_DownloadFromGPUBuffer(this->copyPass, &region, &transferInfo);
}
void RenderPass::BindGraphicsPipeline(Ptr<px::GraphicsPipeline> pipeline) {
  SDL_BindGPUGraphicsPipeline(
      this->renderPass, DownCast<GraphicsPipeline>(pipeline)->GetNative());
}
void RenderPass::BindVertexBuffers(uint32 startSlot,
                                   const Array<BufferBinding> &bindings) {
  Array<SDL_GPUBufferBinding> bufferBindings(allocator);
  bufferBindings.resize(bindings.size());
  for (int i = 0; i < bindings.size(); ++i) {
    bufferBindings[i] = {};
    bufferBindings[i].buffer =
        DownCast<Buffer>(bindings[i].buffer)->GetNative();
    bufferBindings[i].offset = bindings[i].offset;
  }
  SDL_BindGPUVertexBuffers(this->renderPass, startSlot, bufferBindings.data(),
                           bufferBindings.size());
}
void RenderPass::BindIndexBuffer(const BufferBinding &binding,
                                 IndexElementSize indexElementSize) {
  SDL_GPUBufferBinding bufferBinding = {};
  bufferBinding.buffer = DownCast<Buffer>(binding.buffer)->GetNative();
  bufferBinding.offset = binding.offset;
  switch (indexElementSize) {
  case IndexElementSize::Uint16:
    SDL_BindGPUIndexBuffer(
        this->renderPass, &bufferBinding,
        SDL_GPUIndexElementSize::SDL_GPU_INDEXELEMENTSIZE_16BIT);
    break;
  case IndexElementSize::Uint32:
    SDL_BindGPUIndexBuffer(
        this->renderPass, &bufferBinding,
        SDL_GPUIndexElementSize::SDL_GPU_INDEXELEMENTSIZE_32BIT);
    break;
  default:
    assert(false && "Invalid index element size");
  }
}
void RenderPass::BindFragmentSamplers(
    uint32 startSlot, const Array<TextureSamplerBinding> &bindings) {
  Array<SDL_GPUTextureSamplerBinding> samplerBindings(allocator);
  samplerBindings.resize(bindings.size());
  for (int i = 0; i < samplerBindings.size(); ++i) {
    samplerBindings[i] = {};
    samplerBindings[i].sampler =
        DownCast<Sampler>(bindings[i].sampler)->GetNative();
    samplerBindings[i].texture =
        DownCast<Texture>(bindings[i].texture)->GetNative();
  }
  SDL_BindGPUFragmentSamplers(this->renderPass, startSlot,
                              samplerBindings.data(), samplerBindings.size());
}
void RenderPass::SetViewport(const Viewport &viewport) {
  SDL_GPUViewport vp = {viewport.x,      viewport.y,        viewport.width,
                        viewport.height, viewport.minDepth, viewport.maxDepth};
  SDL_SetGPUViewport(this->renderPass, &vp);
}
void RenderPass::SetScissor(uint32 x, uint32 y, uint32 width, uint32 height) {
  SDL_Rect rect = {x, y, width, height};
  SDL_SetGPUScissor(this->renderPass, &rect);
}
void RenderPass::DrawPrimitives(uint32 vertexCount, uint32 instanceCount,
                                uint32 firstVertex, uint32 firstInstance) {
  SDL_DrawGPUPrimitives(this->renderPass, vertexCount, instanceCount,
                        firstVertex, firstInstance);
}
void RenderPass::DrawIndexedPrimitives(uint32 indexCount, uint32 instanceCount,
                                       uint32 firstIndex, uint32 vertexOffset,
                                       uint32 firstInstance) {
  SDL_DrawGPUIndexedPrimitives(this->renderPass, indexCount, instanceCount,
                               firstIndex, vertexOffset, firstInstance);
}
Ptr<px::CopyPass> CommandBuffer::BeginCopyPass() {
  auto *pass = SDL_BeginGPUCopyPass(this->commandBuffer);
  return MakePtr<CopyPass>(GetCreateInfo().allocator, GetCreateInfo().allocator,
                           *this, pass);
}
void CommandBuffer::EndCopyPass(Ptr<px::CopyPass> copyPass) {
  SDL_EndGPUCopyPass(DownCast<CopyPass>(copyPass)->GetNative());
}
Ptr<px::RenderPass>
CommandBuffer::BeginRenderPass(const Array<ColorTargetInfo> &infos) {
  Array<SDL_GPUColorTargetInfo> colorTargetInfos(GetCreateInfo().allocator);
  colorTargetInfos.resize(infos.size());
  for (int i = 0; i < infos.size(); ++i) {
    colorTargetInfos[i] = {};
    colorTargetInfos[i].texture =
        DownCast<Texture>(infos[i].texture)->GetNative();
    colorTargetInfos[i].load_op = convert::LoadOpFrom(infos[i].loadOp);
    colorTargetInfos[i].store_op = convert::StoreOpFrom(infos[i].storeOp);
    colorTargetInfos[i].clear_color = {0, 0, 0, 0};
  }
  auto *renderPass = SDL_BeginGPURenderPass(
      commandBuffer, colorTargetInfos.data(), colorTargetInfos.size(), NULL);
  return MakePtr<RenderPass>(GetCreateInfo().allocator,
                             GetCreateInfo().allocator, *this, renderPass);
}
void CommandBuffer::EndRenderPass(Ptr<px::RenderPass> renderPass) {
  SDL_EndGPURenderPass(DownCast<RenderPass>(renderPass)->GetNative());
}
void CommandBuffer::PushVertexUniformData(uint32 slot, const void *data,
                                          size_t size) {
  SDL_PushGPUVertexUniformData(this->commandBuffer, slot, data, size);
}
GraphicsPipeline::~GraphicsPipeline() {
  SDL_ReleaseGPUGraphicsPipeline(device.GetNative(), pipeline);
}
Device::~Device() {
  if (window)
    SDL_ReleaseWindowFromGPUDevice(device, window);
  SDL_DestroyGPUDevice(device);
}
void Device::ClaimWindow(void *window) {
  if (!SDL_ClaimWindowForGPUDevice(device, static_cast<SDL_Window *>(window))) {
    std::cout << "Failed to claim window for GPU device" << std::endl;
  }
  this->window = static_cast<SDL_Window *>(window);
}

Ptr<px::TransferBuffer>
Device::CreateTransferBuffer(const TransferBuffer::CreateInfo &createInfo) {
  SDL_GPUTransferBufferCreateInfo stagingTextureBufferCI{};
  stagingTextureBufferCI.usage =
      convert::TransferBufferUsageFrom(createInfo.usage);
  stagingTextureBufferCI.size = createInfo.size;

  SDL_GPUTransferBuffer *stagingTextureBuffer =
      SDL_CreateGPUTransferBuffer(device, &stagingTextureBufferCI);
  return MakePtr<TransferBuffer>(createInfo.allocator, createInfo, *this,
                                 stagingTextureBuffer);
}

Ptr<px::Buffer> Device::CreateBuffer(const Buffer::CreateInfo &createInfo) {
  SDL_GPUBufferCreateInfo bufferCI{};
  bufferCI.usage = convert::BufferUsageFrom(createInfo.usage);
  bufferCI.size = createInfo.size;
  SDL_GPUBuffer *buffer = SDL_CreateGPUBuffer(device, &bufferCI);
  return MakePtr<Buffer>(createInfo.allocator, createInfo, *this, buffer);
}

Ptr<px::Texture> Device::CreateTexture(const Texture::CreateInfo &createInfo) {
  SDL_GPUTextureCreateInfo textureCreateInfo = {
      .type = convert::TextureTypeFrom(createInfo.type),
      .format = convert::TextureFormatFrom(createInfo.format),
      .usage = convert::TextureUsageFrom(createInfo.usage),
      .width = createInfo.width,
      .height = createInfo.height,
      .layer_count_or_depth = createInfo.layerCountOrDepth,
      .num_levels = createInfo.numLevels,
      .sample_count = convert::SampleCountFrom(createInfo.sampleCount),
  };

  SDL_GPUTexture *texture = SDL_CreateGPUTexture(device, &textureCreateInfo);
  return MakePtr<Texture>(createInfo.allocator, createInfo, *this, texture,
                          false);
}
Ptr<px::Sampler> Device::CreateSampler(const Sampler::CreateInfo &createInfo) {
  SDL_GPUSamplerCreateInfo samplerCreateInfo = {
      .min_filter = convert::FilterFrom(createInfo.minFilter),
      .mag_filter = convert::FilterFrom(createInfo.magFilter),
      .mipmap_mode = convert::MipmapModeFrom(createInfo.mipmapMode),
      .address_mode_u = convert::AddressModeFrom(createInfo.addressModeU),
      .address_mode_v = convert::AddressModeFrom(createInfo.addressModeV),
      .address_mode_w = convert::AddressModeFrom(createInfo.addressModeW),
  };
  SDL_GPUSampler *sampler = SDL_CreateGPUSampler(device, &samplerCreateInfo);
  return MakePtr<Sampler>(createInfo.allocator, createInfo, *this, sampler);
}

TransferBuffer::~TransferBuffer() {
  SDL_ReleaseGPUTransferBuffer(device.GetNative(), transferBuffer);
}

void *TransferBuffer::Map() {
  return SDL_MapGPUTransferBuffer(device.GetNative(), this->transferBuffer,
                                  false);
}
void TransferBuffer::Unmap() {
  SDL_UnmapGPUTransferBuffer(device.GetNative(), this->transferBuffer);
}

Buffer::~Buffer() { SDL_ReleaseGPUBuffer(device.GetNative(), buffer); }

Ptr<px::Shader> Device::CreateShader(const Shader::CreateInfo &createInfo) {
  SDL_GPUShaderCreateInfo shaderCI = {};
  shaderCI.stage = createInfo.stage == ShaderStage::Vertex
                       ? SDL_GPU_SHADERSTAGE_VERTEX
                       : SDL_GPU_SHADERSTAGE_FRAGMENT;
  shaderCI.code_size = createInfo.size;
  shaderCI.code = reinterpret_cast<const Uint8 *>(createInfo.data);
  shaderCI.format = SDL_GPU_SHADERFORMAT_SPIRV;
  shaderCI.entrypoint = createInfo.entrypoint;
  shaderCI.num_samplers = createInfo.numSamplers;
  shaderCI.num_storage_textures = createInfo.numStorageTextures;
  shaderCI.num_uniform_buffers = createInfo.numUniformBuffers;

  auto *shader = SDL_CreateGPUShader(device, &shaderCI);
  return MakePtr<Shader>(createInfo.allocator, createInfo, *this, shader);
}
Ptr<px::CommandBuffer>
Device::CreateCommandBuffer(const CommandBuffer::CreateInfo &createInfo) {
  SDL_GPUCommandBuffer *commandBuffer = SDL_AcquireGPUCommandBuffer(device);
  return MakePtr<CommandBuffer>(createInfo.allocator, createInfo,
                                commandBuffer);
}

Ptr<px::GraphicsPipeline>
Device::CreateGraphicsPipeline(const GraphicsPipeline::CreateInfo &createInfo) {
  SDL_GPUGraphicsPipelineCreateInfo pipelineCI = {};
  pipelineCI.vertex_shader =
      DownCast<Shader>(createInfo.vertexShader)->GetNative();
  pipelineCI.fragment_shader =
      DownCast<Shader>(createInfo.fragmentShader)->GetNative();
  pipelineCI.rasterizer_state.cull_mode =
      convert::CullModeFrom(createInfo.rasterizerState.cullMode);
  pipelineCI.rasterizer_state.front_face =
      convert::FrontFaceFrom(createInfo.rasterizerState.frontFace);
  pipelineCI.rasterizer_state.depth_bias_constant_factor =
      createInfo.rasterizerState.depthBiasConstantFactor;
  pipelineCI.rasterizer_state.depth_bias_clamp =
      createInfo.rasterizerState.depthBiasClamp;
  pipelineCI.rasterizer_state.depth_bias_slope_factor =
      createInfo.rasterizerState.depthBiasSlopeFactor;
  pipelineCI.rasterizer_state.enable_depth_bias =
      createInfo.rasterizerState.enableDepthBias;
  pipelineCI.rasterizer_state.enable_depth_clip =
      createInfo.rasterizerState.enableDepthClip;
  pipelineCI.primitive_type =
      convert::PrimitiveTypeFrom(createInfo.primitiveType);

  auto numColorTargets = createInfo.targetInfo.colorTargetDescriptions.size();
  pipelineCI.target_info.num_color_targets = numColorTargets;

  Array<SDL_GPUColorTargetDescription> colorTargetDescs(createInfo.allocator);
  colorTargetDescs.resize(numColorTargets);
  for (int i = 0; i < numColorTargets; ++i) {
    SDL_GPUColorTargetDescription colorTargetDesc{};
    colorTargetDesc.format = convert::TextureFormatFrom(
        createInfo.targetInfo.colorTargetDescriptions[i].format);
    auto &pxBlend = createInfo.targetInfo.colorTargetDescriptions[i].blendState;
    auto &blend = colorTargetDesc.blend_state;

    blend.src_alpha_blendfactor =
        convert::BlendFactorFrom(pxBlend.srcColorBlendFactor);
    blend.dst_alpha_blendfactor =
        convert::BlendFactorFrom(pxBlend.dstColorBlendFactor);
    blend.color_blend_op = convert::BlendOpFrom(pxBlend.colorBlendOp);
    blend.src_alpha_blendfactor =
        convert::BlendFactorFrom(pxBlend.srcAlphaBlendFactor);
    blend.dst_alpha_blendfactor =
        convert::BlendFactorFrom(pxBlend.dstAlphaBlendFactor);
    blend.alpha_blend_op = convert::BlendOpFrom(pxBlend.alphaBlendOp);
    blend.color_write_mask = pxBlend.colorWriteMask;
    blend.enable_blend = pxBlend.enableBlend;
    blend.enable_color_write_mask = pxBlend.enableColorWriteMask;
    colorTargetDescs[i] = colorTargetDesc;
  }
  pipelineCI.target_info.color_target_descriptions = colorTargetDescs.data();
  pipelineCI.target_info.num_color_targets = colorTargetDescs.size();
  pipelineCI.vertex_input_state.num_vertex_attributes =
      createInfo.vertexInputState.vertexAttributes.size();
  pipelineCI.vertex_input_state.num_vertex_buffers =
      createInfo.vertexInputState.vertexBufferDescriptions.size();
  Array<SDL_GPUVertexAttribute> vertexAttributes(createInfo.allocator);
  for (int i = 0; i < createInfo.vertexInputState.vertexAttributes.size();
       ++i) {
    SDL_GPUVertexAttribute vertexAttribute = {};
    vertexAttribute.location =
        createInfo.vertexInputState.vertexAttributes[i].location;
    vertexAttribute.buffer_slot =
        createInfo.vertexInputState.vertexAttributes[i].bufferSlot;
    vertexAttribute.format = convert::VertexElementFormatFrom(
        createInfo.vertexInputState.vertexAttributes[i].format);
    vertexAttribute.offset =
        createInfo.vertexInputState.vertexAttributes[i].offset;
    vertexAttributes.push_back(vertexAttribute);
  }
  pipelineCI.vertex_input_state.vertex_attributes = vertexAttributes.data();
  Array<SDL_GPUVertexBufferDescription> vbDescs(createInfo.allocator);
  for (int i = 0;
       i < createInfo.vertexInputState.vertexBufferDescriptions.size(); i++) {
    auto &desc = createInfo.vertexInputState.vertexBufferDescriptions[i];
    SDL_GPUVertexBufferDescription vbDesc = {};
    vbDesc.input_rate = convert::VertexInputRateFrom(desc.inputRate);
    vbDesc.instance_step_rate = desc.instanceStepRate;
    vbDesc.pitch = desc.pitch;
    vbDesc.slot = desc.slot;
    vbDescs.push_back(vbDesc);
  }
  pipelineCI.vertex_input_state.vertex_buffer_descriptions = vbDescs.data();

  auto *pipeline = SDL_CreateGPUGraphicsPipeline(device, &pipelineCI);
  return MakePtr<GraphicsPipeline>(createInfo.allocator, createInfo, *this,
                                   pipeline);
}
Ptr<px::ComputePipeline>
Device::CreateComputePipeline(const ComputePipeline::CreateInfo &createInfo) {
  return MakePtr<ComputePipeline>(createInfo.allocator, createInfo, nullptr);
}
void Device::SubmitCommandBuffer(Ptr<px::CommandBuffer> commandBuffer) {
  SDL_SubmitGPUCommandBuffer(
      DownCast<CommandBuffer>(commandBuffer)->GetNative());
}
Ptr<px::Texture>
Device::AcquireSwapchainTexture(Ptr<px::CommandBuffer> commandBuffer) {

  auto raw = DownCast<CommandBuffer>(commandBuffer);
  SDL_GPUTexture *nativeTex;
  SDL_AcquireGPUSwapchainTexture(raw->GetNative(), window, &nativeTex, nullptr,
                                 nullptr);

  Texture::CreateInfo ci{};
  ci.allocator = commandBuffer->GetCreateInfo().allocator;
  auto texture = MakePtr<Texture>(commandBuffer->GetCreateInfo().allocator, ci,
                                  *this, nativeTex, true);
  return texture;
}
String Device::GetDriver() const { return SDL_GetGPUDeviceDriver(device); }
Texture::~Texture() {
  if (!isSwapchainTexture)
    SDL_ReleaseGPUTexture(device.GetNative(), texture);
}

Shader::~Shader() { SDL_ReleaseGPUShader(device.GetNative(), shader); }
Sampler::~Sampler() { SDL_ReleaseGPUSampler(device.GetNative(), sampler); }
} // namespace paranoixa::sdlgpu
#endif // EMSCRIPTEN