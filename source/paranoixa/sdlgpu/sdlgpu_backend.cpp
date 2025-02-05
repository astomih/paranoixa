#ifndef EMSCRIPTEN
#include "sdlgpu_backend.hpp"
#include "sdlgpu_convert.hpp"

#include <SDL3/SDL.h>
#include <SDL3/SDL_gpu.h>

namespace paranoixa {
Ptr<Device> SDLGPUBackend::CreateDevice(const Device::CreateInfo &createInfo) {
  SDL_GPUDevice *device = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV,
                                              createInfo.debugMode, nullptr);
  if (!device) {
    std::cout << "Failed to create GPU device" << std::endl;
  }
  return MakePtr<SDLGPUDevice>(createInfo.allocator, createInfo, device);
}
void SDLGPUCopyPass::UploadTexture(const TextureTransferInfo &src,
                                   const TextureRegion &dst, bool cycle) {
  SDL_GPUTextureTransferInfo transferInfo = {
      .transfer_buffer =
          DownCast<SDLGPUTransferBuffer>(src.transferBuffer)->GetNative(),
      .offset = src.offset,
  };
  SDL_GPUTextureRegion region = {
      .texture = DownCast<SDLGPUTexture>(dst.texture)->GetNative(),
      .x = dst.x,
      .y = dst.y,
      .z = dst.z,
      .w = dst.width,
      .h = dst.height,
      .d = dst.depth,
  };
  SDL_UploadToGPUTexture(this->copyPass, &transferInfo, &region, cycle);
}
void SDLGPUCopyPass::DownloadTexture(const TextureRegion &src,
                                     const TextureTransferInfo &dst,
                                     bool cycle) {}
void SDLGPUCopyPass::UploadBuffer(const BufferTransferInfo &src,
                                  const BufferRegion &dst, bool cycle) {
  SDL_GPUTransferBufferLocation transferInfo = {
      .transfer_buffer =
          DownCast<SDLGPUTransferBuffer>(src.transferBuffer)->GetNative(),
      .offset = src.offset};
  SDL_GPUBufferRegion region = {
      .buffer = DownCast<SDLGPUBuffer>(dst.buffer)->GetNative(),
      .offset = dst.offset,
      .size = dst.size};
  SDL_UploadToGPUBuffer(this->copyPass, &transferInfo, &region, cycle);
}
void SDLGPUCopyPass::DownloadBuffer(const BufferRegion &src,
                                    const BufferTransferInfo &dst, bool cycle) {
}
void SDLGPURenderPass::BindGraphicsPipeline(Ptr<GraphicsPipeline> pipeline) {
  SDL_BindGPUGraphicsPipeline(
      this->renderPass,
      DownCast<SDLGPUGraphicsPipeline>(pipeline)->GetNative());
}
void SDLGPURenderPass::BindVertexBuffers(uint32 startSlot,
                                         const Array<BufferBinding> &bindings) {
  Array<SDL_GPUBufferBinding> bufferBindings(allocator);
  bufferBindings.resize(bindings.size());
  for (int i = 0; i < bindings.size(); ++i) {
    bufferBindings[i] = {};
    bufferBindings[i].buffer =
        DownCast<SDLGPUBuffer>(bindings[i].buffer)->GetNative();
    bufferBindings[i].offset = bindings[i].offset;
  }
  SDL_BindGPUVertexBuffers(this->renderPass, startSlot, bufferBindings.data(),
                           bufferBindings.size());
}
void SDLGPURenderPass::BindFragmentSamplers(
    uint32 startSlot, const Array<TextureSamplerBinding> &bindings) {
  Array<SDL_GPUTextureSamplerBinding> samplerBindings(allocator);
  samplerBindings.resize(bindings.size());
  for (int i = 0; i < samplerBindings.size(); ++i) {
    samplerBindings[i] = {};
    samplerBindings[i].sampler =
        DownCast<SDLGPUSampler>(bindings[i].sampler)->GetNative();
    samplerBindings[i].texture =
        DownCast<SDLGPUTexture>(bindings[i].texture)->GetNative();
  }
  SDL_BindGPUFragmentSamplers(this->renderPass, startSlot,
                              samplerBindings.data(), samplerBindings.size());
}
void SDLGPURenderPass::DrawPrimitives(uint32 vertexCount, uint32 instanceCount,
                                      uint32 firstVertex,
                                      uint32 firstInstance) {
  SDL_DrawGPUPrimitives(this->renderPass, vertexCount, instanceCount,
                        firstVertex, firstInstance);
}
Ptr<CopyPass> SDLGPUCommandBuffer::BeginCopyPass() {
  auto *pass = SDL_BeginGPUCopyPass(this->commandBuffer);
  return MakePtr<SDLGPUCopyPass>(GetCreateInfo().allocator,
                                 GetCreateInfo().allocator, *this, pass);
}
void SDLGPUCommandBuffer::EndCopyPass(Ptr<CopyPass> copyPass) {
  SDL_EndGPUCopyPass(DownCast<SDLGPUCopyPass>(copyPass)->GetNative());
}
Ptr<RenderPass> SDLGPUCommandBuffer::BeginRenderPass(
    const Array<RenderPass::ColorTargetInfo> &infos) {
  Array<SDL_GPUColorTargetInfo> colorTargetInfos(GetCreateInfo().allocator);
  colorTargetInfos.resize(infos.size());
  for (int i = 0; i < infos.size(); ++i) {
    colorTargetInfos[i] = {};
    colorTargetInfos[i].texture =
        DownCast<SDLGPUTexture>(infos[i].texture)->GetNative();
    colorTargetInfos[i].load_op = convert::LoadOpFrom(infos[i].loadOp);
    colorTargetInfos[i].store_op = convert::StoreOpFrom(infos[i].storeOp);
    colorTargetInfos[i].clear_color = {0, 0, 0, 0};
  }
  auto *renderPass = SDL_BeginGPURenderPass(
      commandBuffer, colorTargetInfos.data(), colorTargetInfos.size(), NULL);
  return MakePtr<SDLGPURenderPass>(
      GetCreateInfo().allocator, GetCreateInfo().allocator, *this, renderPass);
}
void SDLGPUCommandBuffer::EndRenderPass(Ptr<RenderPass> renderPass) {
  SDL_EndGPURenderPass(DownCast<SDLGPURenderPass>(renderPass)->GetNative());
}
SDLGPUGraphicsPipeline::~SDLGPUGraphicsPipeline() {
  SDL_ReleaseGPUGraphicsPipeline(device.GetNative(), pipeline);
}
SDLGPUDevice::~SDLGPUDevice() {
  if (window)
    SDL_ReleaseWindowFromGPUDevice(device, window);
  SDL_DestroyGPUDevice(device);
}
void SDLGPUDevice::ClaimWindow(void *window) {
  if (!SDL_ClaimWindowForGPUDevice(device, static_cast<SDL_Window *>(window))) {
    std::cout << "Failed to claim window for GPU device" << std::endl;
  }
  this->window = static_cast<SDL_Window *>(window);
}

Ptr<TransferBuffer> SDLGPUDevice::CreateTransferBuffer(
    const TransferBuffer::CreateInfo &createInfo) {
  SDL_GPUTransferBufferCreateInfo stagingTextureBufferCI{};
  stagingTextureBufferCI.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
  stagingTextureBufferCI.size = createInfo.size;

  SDL_GPUTransferBuffer *stagingTextureBuffer =
      SDL_CreateGPUTransferBuffer(device, &stagingTextureBufferCI);
  return MakePtr<SDLGPUTransferBuffer>(createInfo.allocator, createInfo, *this,
                                       stagingTextureBuffer);
}

Ptr<Buffer> SDLGPUDevice::CreateBuffer(const Buffer::CreateInfo &createInfo) {
  SDL_GPUBufferCreateInfo bufferCI{};
  bufferCI.usage = SDL_GPU_BUFFERUSAGE_VERTEX;
  bufferCI.size = createInfo.size;
  SDL_GPUBuffer *buffer = SDL_CreateGPUBuffer(device, &bufferCI);
  return MakePtr<SDLGPUBuffer>(createInfo.allocator, createInfo, *this, buffer);
}

Ptr<Texture>
SDLGPUDevice::CreateTexture(const Texture::CreateInfo &createInfo) {
  SDL_GPUTextureCreateInfo textureCreateInfo = {
      .type = SDL_GPU_TEXTURETYPE_2D,
      .format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM,
      .usage = SDL_GPU_TEXTUREUSAGE_SAMPLER,
      .width = createInfo.width,
      .height = createInfo.height,
      .layer_count_or_depth = createInfo.layerCountOrDepth,
      .num_levels = createInfo.numLevels,
      .sample_count = SDL_GPU_SAMPLECOUNT_1};

  SDL_GPUTexture *texture = SDL_CreateGPUTexture(device, &textureCreateInfo);
  return MakePtr<SDLGPUTexture>(createInfo.allocator, createInfo, *this,
                                texture, false);
}
Ptr<Sampler>
SDLGPUDevice::CreateSampler(const Sampler::CreateInfo &createInfo) {
  SDL_GPUSamplerCreateInfo samplerCreateInfo = {
      .min_filter = SDL_GPU_FILTER_LINEAR,
      .mag_filter = SDL_GPU_FILTER_LINEAR,
      .mipmap_mode = SDL_GPU_SAMPLERMIPMAPMODE_NEAREST,
      .address_mode_u = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
      .address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
      .address_mode_w = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
  };
  SDL_GPUSampler *sampler = SDL_CreateGPUSampler(device, &samplerCreateInfo);
  return MakePtr<SDLGPUSampler>(createInfo.allocator, createInfo, *this,
                                sampler);
}

SDLGPUTransferBuffer::~SDLGPUTransferBuffer() {
  SDL_ReleaseGPUTransferBuffer(device.GetNative(), transferBuffer);
}

void *SDLGPUTransferBuffer::Map() {
  return SDL_MapGPUTransferBuffer(device.GetNative(), this->transferBuffer,
                                  false);
}
void SDLGPUTransferBuffer::Unmap() {
  SDL_UnmapGPUTransferBuffer(device.GetNative(), this->transferBuffer);
}

SDLGPUBuffer::~SDLGPUBuffer() {
  SDL_ReleaseGPUBuffer(device.GetNative(), buffer);
}

Ptr<Shader> SDLGPUDevice::CreateShader(const Shader::CreateInfo &createInfo) {
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
  return MakePtr<SDLGPUShader>(createInfo.allocator, createInfo, *this, shader);
}
Ptr<CommandBuffer>
SDLGPUDevice::CreateCommandBuffer(const CommandBuffer::CreateInfo &createInfo) {
  SDL_GPUCommandBuffer *commandBuffer = SDL_AcquireGPUCommandBuffer(device);
  return MakePtr<SDLGPUCommandBuffer>(createInfo.allocator, createInfo,
                                      commandBuffer);
}

Ptr<GraphicsPipeline> SDLGPUDevice::CreateGraphicsPipeline(
    const GraphicsPipeline::CreateInfo &createInfo) {
  SDL_GPUGraphicsPipelineCreateInfo pipelineCI = {};
  pipelineCI.vertex_shader =
      DownCast<SDLGPUShader>(createInfo.vertexShader)->GetNative();
  pipelineCI.fragment_shader =
      DownCast<SDLGPUShader>(createInfo.fragmentShader)->GetNative();
  pipelineCI.rasterizer_state.cull_mode =
      convert::CullModeFrom(createInfo.rasterizerState.cullMode);
  pipelineCI.rasterizer_state.front_face =
      convert::FrontFaceFrom(createInfo.rasterizerState.frontFace);
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
  return MakePtr<SDLGPUGraphicsPipeline>(createInfo.allocator, createInfo,
                                         *this, pipeline);
}
Ptr<ComputePipeline> SDLGPUDevice::CreateComputePipeline(
    const ComputePipeline::CreateInfo &createInfo) {
  return MakePtr<SDLGPUComputePipeline>(createInfo.allocator, createInfo,
                                        nullptr);
}
void SDLGPUDevice::SubmitCommandBuffer(Ptr<CommandBuffer> commandBuffer) {
  SDL_SubmitGPUCommandBuffer(
      DownCast<SDLGPUCommandBuffer>(commandBuffer)->GetNative());
}
Ptr<Texture>
SDLGPUDevice::AcquireSwapchainTexture(Ptr<CommandBuffer> commandBuffer) {

  auto raw = DownCast<SDLGPUCommandBuffer>(commandBuffer);
  SDL_GPUTexture *nativeTex;
  SDL_AcquireGPUSwapchainTexture(raw->GetNative(), window, &nativeTex, nullptr,
                                 nullptr);

  SDLGPUTexture::CreateInfo ci{};
  ci.allocator = commandBuffer->GetCreateInfo().allocator;
  auto texture = MakePtr<SDLGPUTexture>(
      commandBuffer->GetCreateInfo().allocator, ci, *this, nativeTex, true);
  return texture;
}
SDLGPUTexture::~SDLGPUTexture() {
  if (!isSwapchainTexture)
    SDL_ReleaseGPUTexture(device.GetNative(), texture);
}

SDLGPUShader::~SDLGPUShader() {
  SDL_ReleaseGPUShader(device.GetNative(), shader);
}
SDLGPUSampler::~SDLGPUSampler() {
  SDL_ReleaseGPUSampler(device.GetNative(), sampler);
}
} // namespace paranoixa
#endif // EMSCRIPTEN